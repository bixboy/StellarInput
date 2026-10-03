#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/StellarInputTypes.h"
#include "EnhancedInputComponent.h"
#include "UObject/WeakInterfacePtr.h"
#include "StellarInputManager.generated.h"

class APlayerController;
class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UStellarInputConfigAsset;
class IStellarInputReceiverInterface;
class UInputAction;

/**
 * AAA Zero-Tick Lightweight UObject Input Manager.
 * Manages:
 *  - Input Mapping Context stack (PushContext, PopContext, priority stack)
 *  - Native and dynamic action event routing by const UInputAction* pointer (Zero String, Zero Tag)
 *  - Registration of IStellarInputReceiverInterface listeners
 */
UCLASS(BlueprintType)
class STELLARINPUT_API UStellarInputManager : public UObject
{
	GENERATED_BODY()

public:
	UStellarInputManager();

	// --- Lifecycle ---

	/**
	 * Initializes the manager with the owning PlayerController and its EnhancedInputComponent.
	 *
	 * @param InPlayerController The owning player controller.
	 * @param InEnhancedInputComponent The player controller's enhanced input component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input")
	void Initialize(APlayerController* InPlayerController, UEnhancedInputComponent* InEnhancedInputComponent);

	/**
	 * Cleans up bindings and clears the context stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input")
	void Deinitialize();

	// --- Context Stack Management ---

	/**
	 * Pushes an input mapping context from a config asset onto the local player's context stack.
	 *
	 * @param InConfig The input configuration data asset.
	 * @param InPriorityOverride Optional priority override (-1 uses InConfig->Priority).
	 * @return A unique handle to pop this context later.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Context")
	FStellarInputContextHandle PushContext(const UStellarInputConfigAsset* InConfig, const int32 InPriorityOverride = -1);

	/**
	 * Removes an active context from the stack using its handle.
	 *
	 * @param InHandle The handle returned by PushContext.
	 * @return True if the context was found and removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Context")
	bool PopContext(const FStellarInputContextHandle& InHandle);

	/**
	 * Removes all active instances of a specific configuration asset from the stack.
	 *
	 * @param InConfig The configuration asset to pop.
	 * @return True if any context was removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Context")
	bool PopContextByConfig(const UStellarInputConfigAsset* InConfig);

	/**
	 * Clears all active input contexts managed by this manager.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Context")
	void ClearContextStack();

	/** Checks whether a context config is currently active on the stack */
	UFUNCTION(BlueprintPure, Category = "Stellar|Input|Context")
	bool HasContext(const UStellarInputConfigAsset* InConfig) const;

	// --- Action Binding & Routing ---

	/**
	 * Binds all actions in an input config to the manager's router.
	 * Automatically routes events to registered receivers and multicast delegates.
	 *
	 * @param InConfig The configuration data asset whose actions should be bound.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Routing")
	void BindConfigActions(const UStellarInputConfigAsset* InConfig);

	/**
	 * Binds a specific UInputAction directly to the router for custom trigger events.
	 *
	 * @param InAction The input action asset to bind.
	 * @param InTriggerEvent The trigger event to listen for.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Routing")
	void BindActionToRouter(const UInputAction* InAction, const ETriggerEvent InTriggerEvent = ETriggerEvent::Triggered);

	/**
	 * Direct typed C++ helper to bind an action directly to a member function.
	 */
	template<class UserClass, typename FuncType>
	void BindAction(const UInputAction* InAction, const ETriggerEvent InTriggerEvent, UserClass* InObject, FuncType InFunc)
	{
		UEnhancedInputComponent* EIC = GetEnhancedInputComponent();
		if (!EIC || !InAction || !InObject)
			return;

		EIC->BindAction(InAction, InTriggerEvent, InObject, InFunc);
	}

	/**
	 * Registers an EnhancedInputComponent (Controller or Pawn) and binds all active actions to it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Routing")
	void RegisterInputComponent(UEnhancedInputComponent* InEnhancedInputComponent);

	/**
	 * Unregisters an EnhancedInputComponent when a pawn is unpossessed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Routing")
	void UnregisterInputComponent(UEnhancedInputComponent* InEnhancedInputComponent);

	// --- Receiver Registration ---

	/**
	 * Registers an object implementing IStellarInputReceiverInterface to receive routed input actions.
	 *
	 * @param InReceiver The listener object implementing IStellarInputReceiverInterface.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Routing")
	void RegisterReceiver(UObject* InReceiver);

	/**
	 * Unregisters an object from receiving routed input actions.
	 *
	 * @param InReceiver The listener object to unregister.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input|Routing")
	void UnregisterReceiver(UObject* InReceiver);

	// --- Getters ---

	UFUNCTION(BlueprintPure, Category = "Stellar|Input")
	APlayerController* GetPlayerController() const { return OwningPlayerController.Get(); }

	UFUNCTION(BlueprintPure, Category = "Stellar|Input")
	UEnhancedInputComponent* GetEnhancedInputComponent() const;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Stellar|Input|Events")
	FOnStellarActionTriggeredDynamic OnActionTriggered;

	FOnStellarActionTriggeredNative OnActionTriggeredNative;

protected:
	/** Internal router callback invoked by Enhanced Input */
	void HandleActionTriggered(const FInputActionValue& InValue, const UInputAction* InAction, const ETriggerEvent InTriggerEvent);

	/** Resolves the local player subsystem associated with the owning PlayerController */
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	/** Binds actions from a configuration data asset to a specific Enhanced Input Component */
	void BindConfigActionsToComponent(const UStellarInputConfigAsset* InConfig, UEnhancedInputComponent* InComponent);

private:
	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwningPlayerController = nullptr;

	UPROPERTY()
	TArray<TWeakObjectPtr<UEnhancedInputComponent>> RegisteredInputComponents;

	UPROPERTY()
	TArray<FStellarActiveContextRecord> ActiveContextStack;

	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> RegisteredReceivers;

	int32 NextHandleId = 1;
};
