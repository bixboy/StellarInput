#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/StellarInputTypes.h"
#include "StellarInputReceiverInterface.generated.h"

class UInputAction;

UINTERFACE(MinimalAPI, BlueprintType)
class UStellarInputReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface implemented by any Actor, Component, or Object wishing to receive
 * routed Stellar input actions in a completely decoupled manner.
 * Identifies actions natively via const UInputAction* pointers (Zero String, Zero Tag).
 */
class STELLARINPUT_API IStellarInputReceiverInterface
{
	GENERATED_BODY()

public:
	// --- Direct UInputAction* API (Zero String, Zero Tag) ---

	/**
	 * Called when an input action triggers.
	 *
	 * @param InAction The Enhanced Input Action asset that triggered (e.g. IA_Move, IA_Jump).
	 * @param InValue The current input value (Axis1D, Axis2D, Axis3D, or Boolean).
	 * @param InTriggerEvent The trigger phase (Triggered, Started, Completed, Ongoing, Canceled).
	 */
	virtual void ReceiveStellarAction(const UInputAction* InAction, const FInputActionValue& InValue, ETriggerEvent InTriggerEvent)
	{
		Execute_ReceiveStellarAction_BP(_getUObject(), InAction, InValue, InTriggerEvent);
	}

	/** Blueprint implementable event counterpart for Blueprints */
	UFUNCTION(BlueprintImplementableEvent, Category = "Stellar|Input", meta = (DisplayName = "Receive Stellar Action"))
	void ReceiveStellarAction_BP(const UInputAction* InAction, const FInputActionValue& InValue, ETriggerEvent InTriggerEvent);

	/** Optional filter: return false if this receiver is currently not interested in this specific action */
	virtual bool CanReceiveAction(const UInputAction* InAction) const
	{
		return true;
	}

	/** Priority order if multiple receivers listen to the same action (higher = received first) */
	virtual int32 GetReceiverPriority() const
	{
		return 0;
	}

	// --- Legacy Tag API (Backward Compatibility) ---

	/** Legacy tag-based handler */
	virtual void ReceiveStellarInputAction(const FGameplayTag& InActionTag, const FInputActionValue& InValue, ETriggerEvent InTriggerEvent)
	{
		Execute_ReceiveStellarInputAction_BP(_getUObject(), InActionTag, InValue, InTriggerEvent);
	}

	/** Legacy Blueprint implementable event */
	UFUNCTION(BlueprintImplementableEvent, Category = "Stellar|Input", meta = (DisplayName = "Receive Stellar Input Action"))
	void ReceiveStellarInputAction_BP(FGameplayTag InActionTag, const FInputActionValue& InValue, ETriggerEvent InTriggerEvent);

	/** Legacy tag filter */
	virtual bool CanReceiveInputTag(const FGameplayTag& InActionTag) const
	{
		return true;
	}
};
