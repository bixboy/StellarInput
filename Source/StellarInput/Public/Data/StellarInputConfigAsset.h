#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/StellarInputTypes.h"
#include "StellarInputConfigAsset.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * Primary Data Asset holding an Input Mapping Context and action mappings.
 * Serves as the single source of truth for context priority, actions, and key mappings.
 */
UCLASS(BlueprintType, Const)
class STELLARINPUT_API UStellarInputConfigAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Input Mapping Context associated with this configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
	TObjectPtr<const UInputMappingContext> MappingContext = nullptr;

	/** Default priority when this context is pushed onto the stack (higher overrides lower) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (ClampMin = "0"))
	int32 Priority = 0;

	/** Human-readable identifier for debugging and profiling (e.g. Locomotion, Vehicle, Builder, UI) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
	FName ContextName = NAME_None;

	// --- Action Query API (Pointer-Based) ---

	/** Checks whether this configuration contains the specified Enhanced Input Action */
	UFUNCTION(BlueprintPure, Category = "Stellar|Input")
	virtual bool ContainsAction(const UInputAction* InAction) const;

	/** Populates all Enhanced Input Actions configured in this asset for automatic router binding */
	virtual void GetAllInputActions(TArray<const UInputAction*>& OutActions) const;
};
