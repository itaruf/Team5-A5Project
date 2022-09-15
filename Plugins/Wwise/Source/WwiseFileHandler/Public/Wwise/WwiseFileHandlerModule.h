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

#include "AkInclude.h"

class FWwiseIOHook;
class IWwiseSoundBankManager;
class IWwiseExternalSourceManager;
class IWwiseMediaManager;

class IWwiseFileHandlerModule : public IModuleInterface
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

	static IWwiseFileHandlerModule* GetModule()
	{
		const auto ModuleName = GetModuleName();
		if (ModuleName.IsNone())
		{
			return nullptr;
		}

		FModuleManager& ModuleManager = FModuleManager::Get();
		IWwiseFileHandlerModule* Result = ModuleManager.GetModulePtr<IWwiseFileHandlerModule>(ModuleName);
		if (UNLIKELY(!Result))
		{
			if (UNLIKELY(IsEngineExitRequested()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping reloading missing WwiseFileHandler module: Exiting."));
			}
			else if (UNLIKELY(!IsInGameThread()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping loading missing WwiseFileHandler module: Not in game thread"));
			}
			else
			{
				UE_LOG(LogLoad, Log, TEXT("Loading WwiseFileHandler module: %s"), *ModuleName.GetPlainNameString());
				Result = ModuleManager.LoadModulePtr<IWwiseFileHandlerModule>(ModuleName);
				if (UNLIKELY(!Result))
				{
					UE_LOG(LogLoad, Fatal, TEXT("Could not load WwiseFileHandler module: %s not found"), *ModuleName.GetPlainNameString());
				}
			}
		}

		return Result;
	}

	virtual IWwiseSoundBankManager* GetSoundBankManager() { return nullptr; }
	virtual IWwiseExternalSourceManager* GetExternalSourceManager() { return nullptr; }
	virtual IWwiseMediaManager* GetMediaManager() { return nullptr; }
	virtual FWwiseIOHook* InstantiateIOHook() { return nullptr; }

private:
	static inline FName GetModuleNameFromConfig()
	{
		FString ModuleName = TEXT("WwiseFileHandler");
		GConfig->GetString(TEXT("Audio"), TEXT("WwiseFileHandlerModuleName"), ModuleName, GEngineIni);
		return FName(ModuleName);
	}
};

class WWISEFILEHANDLER_API FWwiseFileHandlerModule : public IWwiseFileHandlerModule
{
public:
	FWwiseFileHandlerModule();

	IWwiseSoundBankManager* GetSoundBankManager() override;
	IWwiseExternalSourceManager* GetExternalSourceManager() override;
	IWwiseMediaManager* GetMediaManager() override;
	FWwiseIOHook* InstantiateIOHook() override;
};
