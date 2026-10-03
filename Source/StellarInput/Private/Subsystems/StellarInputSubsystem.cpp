#include "Subsystems/StellarInputSubsystem.h"
#include "Data/StellarInputConfigAsset.h"
#include "Interfaces/StellarInputReceiverInterface.h"
#include "Manager/StellarInputManager.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

UStellarInputSubsystem* UStellarInputSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
		return nullptr;

	const ULocalPlayer* LocalPlayer = nullptr;

	if (const APlayerController* PC = Cast<APlayerController>(WorldContextObject))
	{
		LocalPlayer = PC->GetLocalPlayer();
	}
	else if (const APawn* Pawn = Cast<APawn>(WorldContextObject))
	{
		if (const APlayerController* PawnPC = Cast<APlayerController>(Pawn->GetController()))
			LocalPlayer = PawnPC->GetLocalPlayer();
	}
	else if (const ULocalPlayer* LP = Cast<ULocalPlayer>(WorldContextObject))
	{
		LocalPlayer = LP;
	}
	else if (UWorld* World = WorldContextObject->GetWorld())
	{
		if (APlayerController* FirstPC = World->GetFirstPlayerController())
			LocalPlayer = FirstPC->GetLocalPlayer();
	}

	if (!LocalPlayer)
		return nullptr;

	return LocalPlayer->GetSubsystem<UStellarInputSubsystem>();
}

void UStellarInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("UStellarInputSubsystem: Initialized for LocalPlayer %s"), *GetLocalPlayer()->GetName());
}

void UStellarInputSubsystem::Deinitialize()
{
	ClearAllInputConfigs();
	GlobalReceivers.Empty();
	ActiveInputManager.Reset();
	Super::Deinitialize();
}

FStellarInputContextHandle UStellarInputSubsystem::PushInputConfig(const UStellarInputConfigAsset* InConfig, const int32 InPriorityOverride)
{
	FStellarInputContextHandle Handle;
	if (!InConfig || !InConfig->MappingContext)
		return Handle;

	UEnhancedInputLocalPlayerSubsystem* EISubsystem = GetEnhancedInputSubsystem();
	if (!EISubsystem)
		return Handle;

	const int32 PriorityToApply = (InPriorityOverride >= 0) ? InPriorityOverride : InConfig->Priority;
	EISubsystem->AddMappingContext(InConfig->MappingContext, PriorityToApply);

	Handle.HandleId = NextGlobalHandleId++;
	Handle.ContextName = InConfig->ContextName.IsNone() ? InConfig->GetFName() : InConfig->ContextName;

	FStellarActiveContextRecord Record;
	Record.Handle = Handle;
	Record.ConfigAsset = InConfig;
	Record.AppliedPriority = PriorityToApply;
	GlobalContextStack.Add(Record);

	UE_LOG(LogTemp, Log, TEXT("UStellarInputSubsystem: Pushed config '%s' (Priority %d, Handle %d)"),
		*Handle.ContextName.ToString(), PriorityToApply, Handle.HandleId);

	return Handle;
}

bool UStellarInputSubsystem::PopInputConfig(const FStellarInputContextHandle& InHandle)
{
	if (!InHandle.IsValid())
		return false;

	const int32 FoundIndex = GlobalContextStack.IndexOfByPredicate([&](const FStellarActiveContextRecord& Item)
	{
		return Item.Handle == InHandle;
	});

	if (FoundIndex == INDEX_NONE)
		return false;

	const FStellarActiveContextRecord Record = GlobalContextStack[FoundIndex];
	GlobalContextStack.RemoveAt(FoundIndex);

	if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = GetEnhancedInputSubsystem())
	{
		if (Record.ConfigAsset.IsValid() && Record.ConfigAsset->MappingContext)
			EISubsystem->RemoveMappingContext(Record.ConfigAsset->MappingContext);
	}

	UE_LOG(LogTemp, Log, TEXT("UStellarInputSubsystem: Popped config '%s' (Handle %d)"),
		*Record.Handle.ContextName.ToString(), Record.Handle.HandleId);

	return true;
}

bool UStellarInputSubsystem::PopInputConfigByAsset(const UStellarInputConfigAsset* InConfig)
{
	if (!InConfig)
		return false;

	bool bAnyRemoved = false;
	for (int32 Index = GlobalContextStack.Num() - 1; Index >= 0; --Index)
	{
		if (GlobalContextStack[Index].ConfigAsset.Get() == InConfig)
		{
			PopInputConfig(GlobalContextStack[Index].Handle);
			bAnyRemoved = true;
		}
	}

	return bAnyRemoved;
}

void UStellarInputSubsystem::ClearAllInputConfigs()
{
	UEnhancedInputLocalPlayerSubsystem* EISubsystem = GetEnhancedInputSubsystem();

	for (const FStellarActiveContextRecord& Record : GlobalContextStack)
	{
		if (EISubsystem && Record.ConfigAsset.IsValid() && Record.ConfigAsset->MappingContext)
			EISubsystem->RemoveMappingContext(Record.ConfigAsset->MappingContext);
	}

	GlobalContextStack.Empty();
}

void UStellarInputSubsystem::RegisterGlobalReceiver(UObject* InReceiver)
{
	if (!InReceiver)
		return;

	if (!InReceiver->GetClass()->ImplementsInterface(UStellarInputReceiverInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("UStellarInputSubsystem: Receiver '%s' does not implement IStellarInputReceiverInterface!"), *InReceiver->GetName());
		return;
	}

	TWeakInterfacePtr<IStellarInputReceiverInterface> WeakPtr(InReceiver);
	if (!GlobalReceivers.Contains(WeakPtr))
		GlobalReceivers.Add(WeakPtr);
}

void UStellarInputSubsystem::UnregisterGlobalReceiver(UObject* InReceiver)
{
	if (!InReceiver)
		return;

	TWeakInterfacePtr<IStellarInputReceiverInterface> WeakPtr(InReceiver);
	GlobalReceivers.Remove(WeakPtr);
}

void UStellarInputSubsystem::BroadcastAction(const UInputAction* InAction, const FInputActionValue& InValue, const ETriggerEvent InTriggerEvent)
{
	if (!InAction)
		return;

	OnGlobalActionTriggered.Broadcast(InAction, InValue, InTriggerEvent);
	OnGlobalActionTriggeredNative.Broadcast(InAction, InValue, InTriggerEvent);

	for (int32 Index = GlobalReceivers.Num() - 1; Index >= 0; --Index)
	{
		if (!GlobalReceivers[Index].IsValid())
		{
			GlobalReceivers.RemoveAt(Index);
			continue;
		}

		IStellarInputReceiverInterface* Receiver = GlobalReceivers[Index].Get();
		if (Receiver && Receiver->CanReceiveAction(InAction))
			Receiver->ReceiveStellarAction(InAction, InValue, InTriggerEvent);
	}
}

void UStellarInputSubsystem::BroadcastTaggedAction(FGameplayTag InActionTag, const FInputActionValue& InValue, const ETriggerEvent InTriggerEvent)
{
	for (int32 Index = GlobalReceivers.Num() - 1; Index >= 0; --Index)
	{
		if (!GlobalReceivers[Index].IsValid())
		{
			GlobalReceivers.RemoveAt(Index);
			continue;
		}

		IStellarInputReceiverInterface* Receiver = GlobalReceivers[Index].Get();
		if (Receiver && Receiver->CanReceiveInputTag(InActionTag))
			Receiver->ReceiveStellarInputAction(InActionTag, InValue, InTriggerEvent);
	}
}

void UStellarInputSubsystem::SetActiveInputManager(UStellarInputManager* InManager)
{
	ActiveInputManager = InManager;
}

UStellarInputManager* UStellarInputSubsystem::GetActiveInputManager() const
{
	return ActiveInputManager.Get();
}

EStellarInputDeviceType UStellarInputSubsystem::GetCurrentInputDevice() const
{
	return CurrentDeviceType;
}

void UStellarInputSubsystem::SetCurrentInputDevice(const EStellarInputDeviceType InDeviceType)
{
	if (CurrentDeviceType != InDeviceType)
	{
		CurrentDeviceType = InDeviceType;
		OnInputDeviceChanged.Broadcast(CurrentDeviceType);
		UE_LOG(LogTemp, Log, TEXT("UStellarInputSubsystem: Input device changed to %d"), static_cast<uint8>(InDeviceType));
	}
}

UEnhancedInputLocalPlayerSubsystem* UStellarInputSubsystem::GetEnhancedInputSubsystem() const
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
		return nullptr;

	return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}
