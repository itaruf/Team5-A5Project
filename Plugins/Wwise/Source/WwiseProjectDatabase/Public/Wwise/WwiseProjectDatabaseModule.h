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

#pragma once

#include "Modules/ModuleManager.h"
#include "Misc/ConfigCacheIni.h"
#include "AkUEFeatures.h"

class UWwiseProjectDatabase;

class IWwiseProjectDatabaseModule : public IModuleInterface
{
public:
	static FName GetModuleName()
	{
		static const FName ModuleName = GetModuleNameFromConfig();
		return ModuleName;
	}

	/**
	 * Checks to see if this module is loaded and ready.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(GetModuleName());
	}

	static IWwiseProjectDatabaseModule* GetModule()
	{
		const auto ModuleName = GetModuleName();
		if (ModuleName.IsNone())
		{
			return nullptr;
		}

		FModuleManager& ModuleManager = FModuleManager::Get();
		IWwiseProjectDatabaseModule* Result = ModuleManager.GetModulePtr<IWwiseProjectDatabaseModule>(ModuleName);
		if (UNLIKELY(!Result))
		{
			if (UNLIKELY(IsEngineExitRequested()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping reloading missing WwiseProjectDatabase module: Exiting."));
			}
			else if (UNLIKELY(!IsInGameThread()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping loading missing WwiseProjectDatabase module: Not in game thread"));
			}
			else
			{
				UE_LOG(LogLoad, Log, TEXT("Loading WwiseProjectDatabase module: %s"), *ModuleName.GetPlainNameString());
				Result = ModuleManager.LoadModulePtr<IWwiseProjectDatabaseModule>(ModuleName);
				if (UNLIKELY(!Result))
				{
					UE_LOG(LogLoad, Fatal, TEXT("Could not load WwiseProjectDatabase module: %s not found"), *ModuleName.GetPlainNameString());
				}
			}
		}

		return Result;
	}

	static bool IsInACookingCommandlet()
	{
#if UE_5_0_OR_LATER
		return ::IsRunningCookCommandlet() || ::IsRunningCookOnTheFly();
#else
		return IsRunningCommandlet();		// UE4 Wwise Integration assumes all commandlets are loaded bare-bones
#endif
	}

	virtual UWwiseProjectDatabase* GetProjectDatabase() { return nullptr; }


private:
	static FName GetModuleNameFromConfig()
	{
		FString ModuleName = TEXT("WwiseProjectDatabase");
		GConfig->GetString(TEXT("Audio"), TEXT("WwiseProjectDatabaseModuleName"), ModuleName, GEngineIni);
		return FName(ModuleName);
	}
};

class WWISEPROJECTDATABASE_API FWwiseProjectDatabaseModule : public IWwiseProjectDatabaseModule
{
public:
	UWwiseProjectDatabase* GetProjectDatabase() override;
};
