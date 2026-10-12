#include "StellarInputModule.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogStellarInput);

void FStellarInputModule::StartupModule()
{
	UE_LOG(LogStellarInput, Log, TEXT("StellarInputModule: Initialized modular input subsystem."));
}

void FStellarInputModule::ShutdownModule()
{
	UE_LOG(LogStellarInput, Log, TEXT("StellarInputModule: Shutdown complete."));
}

IMPLEMENT_MODULE(FStellarInputModule, StellarInput)
