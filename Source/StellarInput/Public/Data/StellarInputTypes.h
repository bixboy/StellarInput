#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "InputTriggers.h"
#include "StellarInputTypes.generated.h"

class UInputAction;
class UInputMappingContext;
class UStellarInputConfigAsset;

/**
 * Supported hardware input device types for prompt and calibration display.
 */
UENUM(BlueprintType)
enum class EStellarInputDeviceType : uint8
{
	KeyboardMouse UMETA(DisplayName = "Keyboard & Mouse"),
	Gamepad       UMETA(DisplayName = "Gamepad"),
	FlightStick   UMETA(DisplayName = "Flight Stick / HOTAS"),
	Unknown       UMETA(DisplayName = "Unknown")
};

/**
 * Action entry pairing an Enhanced Input Action asset with trigger configuration and optional metadata tag.
 */
USTRUCT(BlueprintType)
struct STELLARINPUT_API FStellarTaggedInputAction
{
	GENERATED_BODY()

	/** Enhanced Input Action asset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<const UInputAction> InputAction = nullptr;

	/** Preferred trigger event for binding (Started, Triggered, Completed, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	ETriggerEvent PreferredTriggerEvent = ETriggerEvent::Triggered;

	/** Optional semantic tag for UI categorization or display metadata */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FGameplayTag InputTag;
};

using FStellarActionEntry = FStellarTaggedInputAction;

/**
 * Lightweight handle representing a pushed input mapping context on the stack.
 */
USTRUCT(BlueprintType)
struct STELLARINPUT_API FStellarInputContextHandle
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	int32 HandleId = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	FName ContextName = NAME_None;

	FORCEINLINE bool IsValid() const
	{
		return HandleId > 0;
	}

	FORCEINLINE void Invalidate()
	{
		HandleId = 0;
		ContextName = NAME_None;
	}

	FORCEINLINE bool operator==(const FStellarInputContextHandle& InOther) const
	{
		return HandleId == InOther.HandleId;
	}

	FORCEINLINE bool operator!=(const FStellarInputContextHandle& InOther) const
	{
		return HandleId != InOther.HandleId;
	}
};

/**
 * Record of an active input mapping context currently applied to the stack.
 */
USTRUCT()
struct STELLARINPUT_API FStellarActiveContextRecord
{
	GENERATED_BODY()

	UPROPERTY()
	FStellarInputContextHandle Handle;

	UPROPERTY()
	TWeakObjectPtr<const UStellarInputConfigAsset> ConfigAsset;

	UPROPERTY()
	int32 AppliedPriority = 0;
};

// --- Action-Based Delegates (Zero String, Zero Tag) ---

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStellarActionTriggeredDynamic, const UInputAction*, Action, const FInputActionValue&, ActionValue, ETriggerEvent, TriggerEvent);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnStellarActionTriggeredNative, const UInputAction*, const FInputActionValue&, ETriggerEvent);

// --- Legacy Tagged Delegates (Compatibility Fallback) ---

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStellarInputActionDynamic, FGameplayTag, ActionTag, const FInputActionValue&, ActionValue, ETriggerEvent, TriggerEvent);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnStellarInputActionNative, const FGameplayTag&, const FInputActionValue&, ETriggerEvent);
