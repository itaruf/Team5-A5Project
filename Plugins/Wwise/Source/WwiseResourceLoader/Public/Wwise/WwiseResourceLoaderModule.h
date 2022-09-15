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

#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "Misc/ConfigCacheIni.h"

class UWwiseResourceLoaderImpl;
class UWwiseResourceLoader;

class IWwiseResourceLoaderModule : public IModuleInterface
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

	static IWwiseResourceLoaderModule* GetModule()
	{
		const auto ModuleName = GetModuleName();
		if (ModuleName.IsNone())
		{
			return nullptr;
		}

		FModuleManager& ModuleManager = FModuleManager::Get();
		IWwiseResourceLoaderModule* Result = ModuleManager.GetModulePtr<IWwiseResourceLoaderModule>(ModuleName);
		if (UNLIKELY(!Result))
		{
			if (UNLIKELY(IsEngineExitRequested()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping reloading missing WwiseResourceLoaderImpl module: Exiting."));
			}
			else if (UNLIKELY(!IsInGameThread()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping loading missing WwiseResourceLoaderImpl module: Not in game thread"));
			}
			else
			{
				UE_LOG(LogLoad, Log, TEXT("Loading WwiseResourceLoaderImpl module: %s"), *ModuleName.GetPlainNameString());
				Result = ModuleManager.LoadModulePtr<IWwiseResourceLoaderModule>(ModuleName);
				if (UNLIKELY(!Result))
				{
					UE_LOG(LogLoad, Fatal, TEXT("Could not load WwiseResourceLoaderImpl module: %s not found"), *ModuleName.GetPlainNameString());
				}
			}
		}

		return Result;
	}

	virtual UWwiseResourceLoader* GetResourceLoader() { return nullptr; }
	virtual UWwiseResourceLoaderImpl* InstantiateResourceLoaderImpl() { return nullptr; }

private:
	static inline FName GetModuleNameFromConfig()
	{
		FString ModuleName = TEXT("WwiseResourceLoader");
		GConfig->GetString(TEXT("Audio"), TEXT("WwiseResourceLoaderModuleName"), ModuleName, GEngineIni);
		return FName(ModuleName);
	}
};

class WWISERESOURCELOADER_API FWwiseResourceLoaderModule : public IWwiseResourceLoaderModule
{
public:
	UWwiseResourceLoader* GetResourceLoader() override;
	UWwiseResourceLoaderImpl* InstantiateResourceLoaderImpl() override;
};
