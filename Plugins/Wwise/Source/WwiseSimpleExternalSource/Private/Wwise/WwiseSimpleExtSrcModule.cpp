/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

#include "Wwise/WwiseSimpleExtSrcModule.h"
#include "Wwise/WwiseFileHandlerModule.h"
#include "Wwise/SimpleExtSrc/WwiseSimpleExtSrcManager.h"
#include "Wwise/Stats/SimpleExtSrc.h"

#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "Wwise/SimpleExtSrc/WwiseExternalSourceSettings.h"
#include "ISettingsModule.h"
#endif


#define LOCTEXT_NAMESPACE "SimpleExternalSourceModule"

IMPLEMENT_MODULE(FWwiseSimpleExtSrcModule, WwiseSimpleExternalSource)

IWwiseExternalSourceManager* FWwiseSimpleExtSrcModule::GetExternalSourceManager()
{
	if (UNLIKELY(!GEngine))
	{
		UE_LOG(LogLoad, Warning, TEXT("Trying to load WwiseExtSrcManager subsystem without an engine."));
		return nullptr;
	}
	return GEngine->GetEngineSubsystem<UWwiseSimpleExtSrcManager>();
}

void FWwiseSimpleExtSrcModule::StartupModule()
{
	FWwiseFileHandlerModule::StartupModule();
#if WITH_EDITOR

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Wwise", "Wwise External Sources",
		                                 LOCTEXT("RuntimeSettingsName", "Wwise Simple External Source Settings"), LOCTEXT("RuntimeSettingsDescription", "Set the external source data tables and staging directory"),
		                                 GetMutableDefault<UWwiseExternalSourceSettings>());
	}
#endif
}

void FWwiseSimpleExtSrcModule::ShutdownModule()
{
	FWwiseFileHandlerModule::ShutdownModule();
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Wwise", "Wwise External Sources");
	}
#endif
}

#undef LOCTEXT_NAMESPACE
