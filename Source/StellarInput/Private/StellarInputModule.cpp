#include "StellarInputModule.h"
#include "Modules/ModuleManager.h"

void FStellarInputModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("StellarInputModule: Initialized modular input subsystem."));
}

void FStellarInputModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("StellarInputModule: Shutdown complete."));
}

IMPLEMENT_MODULE(FStellarInputModule, StellarInput)
