#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Main runtime module interface for the StellarInput plugin.
 */
class FStellarInputModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
