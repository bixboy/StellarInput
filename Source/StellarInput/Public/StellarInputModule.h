#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

STELLARINPUT_API DECLARE_LOG_CATEGORY_EXTERN(LogStellarInput, Log, All);

/**
 * Main runtime module interface for the StellarInput plugin.
 */
class FStellarInputModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
