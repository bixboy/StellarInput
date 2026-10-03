#include "Libraries/StellarInputFunctionLibrary.h"
#include "Subsystems/StellarInputSubsystem.h"
#include "Manager/StellarInputManager.h"
#include "Data/StellarInputConfigAsset.h"

FStellarInputContextHandle UStellarInputFunctionLibrary::PushInputContext(const UObject* WorldContextObject, const UStellarInputConfigAsset* InConfig, const int32 InPriorityOverride)
{
	if (UStellarInputSubsystem* Subsystem = UStellarInputSubsystem::Get(WorldContextObject))
		return Subsystem->PushInputConfig(InConfig, InPriorityOverride);

	return FStellarInputContextHandle();
}

bool UStellarInputFunctionLibrary::PopInputContext(const UObject* WorldContextObject, const FStellarInputContextHandle& InHandle)
{
	if (UStellarInputSubsystem* Subsystem = UStellarInputSubsystem::Get(WorldContextObject))
		return Subsystem->PopInputConfig(InHandle);

	return false;
}

bool UStellarInputFunctionLibrary::PopInputContextByAsset(const UObject* WorldContextObject, const UStellarInputConfigAsset* InConfig)
{
	if (UStellarInputSubsystem* Subsystem = UStellarInputSubsystem::Get(WorldContextObject))
		return Subsystem->PopInputConfigByAsset(InConfig);

	return false;
}

void UStellarInputFunctionLibrary::RegisterInputReceiver(const UObject* WorldContextObject, UObject* InReceiver)
{
	if (UStellarInputSubsystem* Subsystem = UStellarInputSubsystem::Get(WorldContextObject))
		Subsystem->RegisterGlobalReceiver(InReceiver);
}

void UStellarInputFunctionLibrary::UnregisterInputReceiver(const UObject* WorldContextObject, UObject* InReceiver)
{
	if (UStellarInputSubsystem* Subsystem = UStellarInputSubsystem::Get(WorldContextObject))
		Subsystem->UnregisterGlobalReceiver(InReceiver);
}

UStellarInputManager* UStellarInputFunctionLibrary::GetInputManager(const UObject* WorldContextObject)
{
	if (UStellarInputSubsystem* Subsystem = UStellarInputSubsystem::Get(WorldContextObject))
		return Subsystem->GetActiveInputManager();

	return nullptr;
}

EStellarInputDeviceType UStellarInputFunctionLibrary::GetCurrentInputDevice(const UObject* WorldContextObject)
{
	if (UStellarInputSubsystem* Subsystem = UStellarInputSubsystem::Get(WorldContextObject))
		return Subsystem->GetCurrentInputDevice();

	return EStellarInputDeviceType::KeyboardMouse;
}
