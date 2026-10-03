#include "Manager/StellarInputManager.h"
#include "Data/StellarInputConfigAsset.h"
#include "Interfaces/StellarInputReceiverInterface.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "InputAction.h"

UStellarInputManager::UStellarInputManager()
{
}

// --- Lifecycle ---

void UStellarInputManager::Initialize(APlayerController* InPlayerController, UEnhancedInputComponent* InEnhancedInputComponent)
{
	OwningPlayerController = InPlayerController;
	RegisteredInputComponents.Empty();

	if (InEnhancedInputComponent)
		RegisterInputComponent(InEnhancedInputComponent);

	UE_LOG(LogTemp, Log, TEXT("StellarInputManager: Initialized for Controller '%s'"),
		InPlayerController ? *InPlayerController->GetName() : TEXT("None"));
}

void UStellarInputManager::Deinitialize()
{
	ClearContextStack();
	RegisteredReceivers.Empty();
	RegisteredInputComponents.Empty();
	OwningPlayerController.Reset();
}

// --- Context Stack Management ---

FStellarInputContextHandle UStellarInputManager::PushContext(const UStellarInputConfigAsset* InConfig, const int32 InPriorityOverride)
{
	FStellarInputContextHandle Handle;
	if (!InConfig || !InConfig->MappingContext)
		return Handle;

	for (const FStellarActiveContextRecord& Record : ActiveContextStack)
	{
		if (Record.ConfigAsset.Get() == InConfig)
			return Record.Handle;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
	if (!Subsystem)
		return Handle;

	const int32 PriorityToApply = (InPriorityOverride >= 0) ? InPriorityOverride : InConfig->Priority;
	Subsystem->AddMappingContext(InConfig->MappingContext, PriorityToApply);

	Handle.HandleId = NextHandleId++;
	Handle.ContextName = InConfig->ContextName.IsNone() ? InConfig->GetFName() : InConfig->ContextName;

	FStellarActiveContextRecord Record;
	Record.Handle = Handle;
	Record.ConfigAsset = InConfig;
	Record.AppliedPriority = PriorityToApply;
	ActiveContextStack.Add(Record);

	// Automatically bind actions from this config to all registered input components
	BindConfigActions(InConfig);

	UE_LOG(LogTemp, Log, TEXT("StellarInputManager: Pushed context '%s' (Priority %d, Handle %d)"),
		*Handle.ContextName.ToString(), PriorityToApply, Handle.HandleId);

	return Handle;
}

bool UStellarInputManager::PopContext(const FStellarInputContextHandle& InHandle)
{
	if (!InHandle.IsValid())
		return false;

	const int32 FoundIndex = ActiveContextStack.IndexOfByPredicate([&](const FStellarActiveContextRecord& Item)
	{
		return Item.Handle == InHandle;
	});

	if (FoundIndex == INDEX_NONE)
		return false;

	const FStellarActiveContextRecord Record = ActiveContextStack[FoundIndex];
	ActiveContextStack.RemoveAt(FoundIndex);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem())
	{
		if (Record.ConfigAsset.IsValid() && Record.ConfigAsset->MappingContext)
			Subsystem->RemoveMappingContext(Record.ConfigAsset->MappingContext);
	}

	UE_LOG(LogTemp, Log, TEXT("StellarInputManager: Popped context '%s' (Handle %d)"),
		*Record.Handle.ContextName.ToString(), Record.Handle.HandleId);

	return true;
}

bool UStellarInputManager::PopContextByConfig(const UStellarInputConfigAsset* InConfig)
{
	if (!InConfig)
		return false;

	bool bAnyRemoved = false;
	for (int32 Index = ActiveContextStack.Num() - 1; Index >= 0; --Index)
	{
		if (ActiveContextStack[Index].ConfigAsset.Get() == InConfig)
		{
			PopContext(ActiveContextStack[Index].Handle);
			bAnyRemoved = true;
		}
	}

	return bAnyRemoved;
}

void UStellarInputManager::ClearContextStack()
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();

	for (const FStellarActiveContextRecord& Record : ActiveContextStack)
	{
		if (Subsystem && Record.ConfigAsset.IsValid() && Record.ConfigAsset->MappingContext)
			Subsystem->RemoveMappingContext(Record.ConfigAsset->MappingContext);
	}

	ActiveContextStack.Empty();
}

bool UStellarInputManager::HasContext(const UStellarInputConfigAsset* InConfig) const
{
	if (!InConfig)
		return false;

	for (const FStellarActiveContextRecord& Record : ActiveContextStack)
	{
		if (Record.ConfigAsset.Get() == InConfig)
			return true;
	}

	return false;
}

// --- Action Binding & Component Registration ---

void UStellarInputManager::RegisterInputComponent(UEnhancedInputComponent* InEnhancedInputComponent)
{
	if (!InEnhancedInputComponent)
		return;

	if (!RegisteredInputComponents.Contains(InEnhancedInputComponent))
	{
		RegisteredInputComponents.Add(InEnhancedInputComponent);
		UE_LOG(LogTemp, Display, TEXT("StellarInputManager: Registered InputComponent '%s' (Owner: '%s')"),
			*InEnhancedInputComponent->GetName(),
			InEnhancedInputComponent->GetOwner() ? *InEnhancedInputComponent->GetOwner()->GetName() : TEXT("None"));
	}

	// Bind all active context configurations to this newly registered component
	for (const FStellarActiveContextRecord& Record : ActiveContextStack)
	{
		if (Record.ConfigAsset.IsValid())
			BindConfigActionsToComponent(Record.ConfigAsset.Get(), InEnhancedInputComponent);
	}
}

void UStellarInputManager::UnregisterInputComponent(UEnhancedInputComponent* InEnhancedInputComponent)
{
	if (!InEnhancedInputComponent)
		return;

	RegisteredInputComponents.Remove(InEnhancedInputComponent);
}

void UStellarInputManager::BindConfigActions(const UStellarInputConfigAsset* InConfig)
{
	if (!InConfig)
		return;

	for (int32 Index = RegisteredInputComponents.Num() - 1; Index >= 0; --Index)
	{
		if (!RegisteredInputComponents[Index].IsValid())
		{
			RegisteredInputComponents.RemoveAt(Index);
			continue;
		}

		BindConfigActionsToComponent(InConfig, RegisteredInputComponents[Index].Get());
	}
}

void UStellarInputManager::BindConfigActionsToComponent(const UStellarInputConfigAsset* InConfig, UEnhancedInputComponent* InComponent)
{
	if (!InConfig || !InComponent)
		return;

	TArray<const UInputAction*> ActionsToBind;
	InConfig->GetAllInputActions(ActionsToBind);

	for (const UInputAction* Action : ActionsToBind)
	{
		if (!Action)
			continue;

		InComponent->BindAction(Action, ETriggerEvent::Triggered, this,
			&UStellarInputManager::HandleActionTriggered, Action, ETriggerEvent::Triggered);

		InComponent->BindAction(Action, ETriggerEvent::Started, this,
			&UStellarInputManager::HandleActionTriggered, Action, ETriggerEvent::Started);

		InComponent->BindAction(Action, ETriggerEvent::Completed, this,
			&UStellarInputManager::HandleActionTriggered, Action, ETriggerEvent::Completed);

		InComponent->BindAction(Action, ETriggerEvent::Canceled, this,
			&UStellarInputManager::HandleActionTriggered, Action, ETriggerEvent::Canceled);
	}
}

void UStellarInputManager::BindActionToRouter(const UInputAction* InAction, const ETriggerEvent InTriggerEvent)
{
	if (!InAction)
		return;

	for (const TWeakObjectPtr<UEnhancedInputComponent>& CompPtr : RegisteredInputComponents)
	{
		if (CompPtr.IsValid())
		{
			CompPtr->BindAction(InAction, InTriggerEvent, this,
				&UStellarInputManager::HandleActionTriggered, InAction, InTriggerEvent);
		}
	}
}

UEnhancedInputComponent* UStellarInputManager::GetEnhancedInputComponent() const
{
	for (const TWeakObjectPtr<UEnhancedInputComponent>& CompPtr : RegisteredInputComponents)
	{
		if (CompPtr.IsValid())
			return CompPtr.Get();
	}
	return nullptr;
}

// --- Receiver Registration ---

void UStellarInputManager::RegisterReceiver(UObject* InReceiver)
{
	if (!InReceiver)
		return;

	if (!InReceiver->GetClass()->ImplementsInterface(UStellarInputReceiverInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("StellarInputManager: Receiver '%s' does not implement IStellarInputReceiverInterface!"), *InReceiver->GetName());
		return;
	}

	TWeakObjectPtr<UObject> WeakObj(InReceiver);
	if (!RegisteredReceivers.Contains(WeakObj))
	{
		RegisteredReceivers.Add(WeakObj);
		UE_LOG(LogTemp, Display, TEXT("StellarInputManager: Registered Receiver '%s'"), *InReceiver->GetName());
	}
}

void UStellarInputManager::UnregisterReceiver(UObject* InReceiver)
{
	if (!InReceiver)
		return;

	TWeakObjectPtr<UObject> WeakObj(InReceiver);
	RegisteredReceivers.Remove(WeakObj);
}

// --- Internal Router Callback ---

void UStellarInputManager::HandleActionTriggered(const FInputActionValue& InValue, const UInputAction* InAction, const ETriggerEvent InTriggerEvent)
{
	if (!InAction)
		return;

	// 1. Dispatch multicast delegates
	OnActionTriggered.Broadcast(InAction, InValue, InTriggerEvent);
	OnActionTriggeredNative.Broadcast(InAction, InValue, InTriggerEvent);

	// 2. Dispatch to registered receivers
	for (int32 Index = RegisteredReceivers.Num() - 1; Index >= 0; --Index)
	{
		if (!RegisteredReceivers[Index].IsValid())
		{
			RegisteredReceivers.RemoveAt(Index);
			continue;
		}

		UObject* ReceiverObj = RegisteredReceivers[Index].Get();
		if (IStellarInputReceiverInterface* Receiver = Cast<IStellarInputReceiverInterface>(ReceiverObj))
		{
			if (Receiver->CanReceiveAction(InAction))
				Receiver->ReceiveStellarAction(InAction, InValue, InTriggerEvent);
		}
	}
}

// --- Helpers ---

UEnhancedInputLocalPlayerSubsystem* UStellarInputManager::GetEnhancedInputSubsystem() const
{
	if (!OwningPlayerController.IsValid())
		return nullptr;

	ULocalPlayer* LP = OwningPlayerController->GetLocalPlayer();
	if (!LP)
		return nullptr;

	return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}
