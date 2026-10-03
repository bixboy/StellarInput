#include "Data/StellarInputConfigAsset.h"
#include "InputAction.h"
#include "InputMappingContext.h"

void UStellarInputConfigAsset::GetAllInputActions(TArray<const UInputAction*>& OutActions) const
{
}

bool UStellarInputConfigAsset::ContainsAction(const UInputAction* InAction) const
{
	if (!InAction)
		return false;

	TArray<const UInputAction*> AllActions;
	GetAllInputActions(AllActions);
	return AllActions.Contains(InAction);
}
