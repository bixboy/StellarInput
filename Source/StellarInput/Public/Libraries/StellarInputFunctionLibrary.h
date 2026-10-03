#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/StellarInputTypes.h"
#include "StellarInputFunctionLibrary.generated.h"

class UStellarInputConfigAsset;
class UStellarInputManager;

/**
 * Static Blueprint and C++ helper library providing quick access
 * to context stacking and input routing.
 */
UCLASS()
class STELLARINPUT_API UStellarInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Pushes an input mapping configuration onto the local player's context stack.
	 *
	 * @param WorldContextObject Any Actor, Component, or Widget in the world.
	 * @param InConfig The input configuration data asset to push.
	 * @param InPriorityOverride Optional priority override (-1 uses config asset default).
	 * @return A unique handle to pop this context later.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input", meta = (WorldContext = "WorldContextObject"))
	static FStellarInputContextHandle PushInputContext(const UObject* WorldContextObject, const UStellarInputConfigAsset* InConfig, const int32 InPriorityOverride = -1);

	/**
	 * Pops an input mapping configuration from the local player's context stack using its handle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input", meta = (WorldContext = "WorldContextObject"))
	static bool PopInputContext(const UObject* WorldContextObject, const FStellarInputContextHandle& InHandle);

	/**
	 * Pops all instances of a configuration asset from the local player's context stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input", meta = (WorldContext = "WorldContextObject"))
	static bool PopInputContextByAsset(const UObject* WorldContextObject, const UStellarInputConfigAsset* InConfig);

	/**
	 * Registers an Actor, Component, or Object implementing IStellarInputReceiverInterface
	 * to receive routed inputs.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input", meta = (WorldContext = "WorldContextObject"))
	static void RegisterInputReceiver(const UObject* WorldContextObject, UObject* InReceiver);

	/**
	 * Unregisters an object from receiving routed inputs.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stellar|Input", meta = (WorldContext = "WorldContextObject"))
	static void UnregisterInputReceiver(const UObject* WorldContextObject, UObject* InReceiver);

	/**
	 * Gets the active StellarInputManager for the player, if one exists.
	 */
	UFUNCTION(BlueprintPure, Category = "Stellar|Input", meta = (WorldContext = "WorldContextObject"))
	static UStellarInputManager* GetInputManager(const UObject* WorldContextObject);

	/**
	 * Returns the currently active input hardware device for the local player.
	 */
	UFUNCTION(BlueprintPure, Category = "Stellar|Input", meta = (WorldContext = "WorldContextObject"))
	static EStellarInputDeviceType GetCurrentInputDevice(const UObject* WorldContextObject);
};
