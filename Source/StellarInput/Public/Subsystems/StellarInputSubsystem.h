#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Data/StellarInputTypes.h"
#include "UObject/WeakInterfacePtr.h"
#include "StellarInputSubsystem.generated.h"

class UStellarInputConfigAsset;
class IStellarInputReceiverInterface;
class UEnhancedInputLocalPlayerSubsystem;
class UStellarInputManager;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStellarInputDeviceChanged, EStellarInputDeviceType, NewDeviceType);

/**
 * Universal Local Player Subsystem providing cross-plugin input routing,
 * context stack management, and input device detection.
 */
UCLASS()
class STELLARINPUT_API UStellarInputSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	// --- Static Accessor ---

	/** Convenience getter from any world context object (PlayerController, Pawn, Widget) */
	UFUNCTION(BlueprintPure, Category = "Stellar|Input", meta = (WorldContext = "WorldContextObject"))
	static UStellarInputSubsystem* Get(const UObject* WorldContextObject);

	// --- Lifecycle ---

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Global Context Stack ---

	/**
	 * Pushes an input configuration onto the local player's input context stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Context")
	FStellarInputContextHandle PushInputConfig(const UStellarInputConfigAsset* InConfig, const int32 InPriorityOverride = -1);

	/**
	 * Pops an input configuration off the stack using its handle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Context")
	bool PopInputConfig(const FStellarInputContextHandle& InHandle);

	/**
	 * Pops all occurrences of a specific configuration asset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Context")
	bool PopInputConfigByAsset(const UStellarInputConfigAsset* InConfig);

	/**
	 * Clears all active input configurations managed by this subsystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Context")
	void ClearAllInputConfigs();

	// --- Global Action Routing ---

	/**
	 * Registers a global receiver for routed input actions across plugins.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Routing")
	void RegisterGlobalReceiver(UObject* InReceiver);

	/**
	 * Unregisters a global receiver.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Routing")
	void UnregisterGlobalReceiver(UObject* InReceiver);

	/**
	 * Dispatches an action event to all global receivers and delegates.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Routing")
	void BroadcastAction(const UInputAction* InAction, const FInputActionValue& InValue, const ETriggerEvent InTriggerEvent);

	/**
	 * Legacy tagged action broadcast for backward compatibility.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Routing")
	void BroadcastTaggedAction(FGameplayTag InActionTag, const FInputActionValue& InValue, const ETriggerEvent InTriggerEvent);

	// --- Active Input Manager ---

	UFUNCTION(BlueprintCallable, Category = "Stellar|Input")
	void SetActiveInputManager(UStellarInputManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Stellar|Input")
	UStellarInputManager* GetActiveInputManager() const;

	// --- Device Detection ---

	/** Returns the currently active input device (KeyboardMouse, Gamepad, FlightStick) */
	UFUNCTION(BlueprintPure, Category = "Stellar|Input|Device")
	EStellarInputDeviceType GetCurrentInputDevice() const;

	/** Sets the active input device and notifies listeners if changed */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Device")
	void SetCurrentInputDevice(const EStellarInputDeviceType InDeviceType);

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Stellar|Input|Events")
	FOnStellarActionTriggeredDynamic OnGlobalActionTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Stellar|Input|Events")
	FOnStellarInputDeviceChanged OnInputDeviceChanged;

	FOnStellarActionTriggeredNative OnGlobalActionTriggeredNative;

private:
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	UPROPERTY()
	TArray<FStellarActiveContextRecord> GlobalContextStack;

	TArray<TWeakInterfacePtr<IStellarInputReceiverInterface>> GlobalReceivers;

	UPROPERTY()
	TWeakObjectPtr<UStellarInputManager> ActiveInputManager = nullptr;

	EStellarInputDeviceType CurrentDeviceType = EStellarInputDeviceType::KeyboardMouse;

	int32 NextGlobalHandleId = 1;
};
