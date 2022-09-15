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
#include "UObject/Class.h"

struct FWwiseSharedPlatformId;
class UWwiseResourceCooker;

UENUM()
enum class EWwiseExportDebugNameRule
{
	Release,
	Name,
	ObjectPath
};

class IWwiseResourceCookerModule : public IModuleInterface
{
public:
	void WWISERESOURCECOOKER_API StartupModule() override;
	void WWISERESOURCECOOKER_API ShutdownModule() override;

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

	static IWwiseResourceCookerModule* GetModule()
	{
		const auto ModuleName = GetModuleName();
		if (ModuleName.IsNone())
		{
			return nullptr;
		}

		FModuleManager& ModuleManager = FModuleManager::Get();
		IWwiseResourceCookerModule* Result = ModuleManager.GetModulePtr<IWwiseResourceCookerModule>(ModuleName);
		if (UNLIKELY(!Result))
		{
			if (UNLIKELY(IsEngineExitRequested()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping reloading missing WwiseResourceCooker module: Exiting."));
			}
			else if (UNLIKELY(!IsInGameThread()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping loading missing WwiseResourceCooker module: Not in game thread"));
			}
			else
			{
				UE_LOG(LogLoad, Log, TEXT("Loading WwiseResourceCooker module: %s"), *ModuleName.GetPlainNameString());
				Result = ModuleManager.LoadModulePtr<IWwiseResourceCookerModule>(ModuleName);
				if (UNLIKELY(!Result))
				{
					UE_LOG(LogLoad, Fatal, TEXT("Could not load WwiseResourceCooker module: %s not found"), *ModuleName.GetPlainNameString());
				}
			}
		}

		return Result;
	}

	virtual UWwiseResourceCooker* GetDefaultCooker()
	{
		return nullptr;
	}

	virtual UWwiseResourceCooker* CreateCookerForPlatform(
		const ITargetPlatform* TargetPlatform,
		const FWwiseSharedPlatformId& InPlatform,
		EWwiseExportDebugNameRule InExportDebugNameRule = EWwiseExportDebugNameRule::Release)
	{
		return nullptr;
	}

	virtual void DestroyCookerForPlatform(const ITargetPlatform* TargetPlatform)
	{
	}

	virtual UWwiseResourceCooker* GetCookerForPlatform(const ITargetPlatform* TargetPlatform)
	{
		return nullptr;
	}

	UWwiseResourceCooker* GetCookerForArchive(const FArchive& InArchive)
	{
		if (!InArchive.IsCooking() || !InArchive.IsSaving())
		{
			return nullptr;
		}

		return GetCookerForPlatform(InArchive.CookingTarget());
	}

	virtual void DestroyAllCookerPlatforms()
	{
	}

protected:
	static WWISERESOURCECOOKER_API FDelegateHandle ModifyCookDelegateHandle;

	virtual void OnModifyCook(TConstArrayView<const ITargetPlatform*> InTargetPlatforms, TArray<FName>& InOutPackagesToCook, TArray<FName>& InOutPackagesToNeverCook)
	{
	}

private:
	static inline FName GetModuleNameFromConfig()
	{
		FString ModuleName = TEXT("WwiseResourceCooker");
		GConfig->GetString(TEXT("Audio"), TEXT("WwiseResourceCookerModuleName"), ModuleName, GEngineIni);
		return FName(ModuleName);
	}
};

class WWISERESOURCECOOKER_API FWwiseResourceCookerModuleImpl : public IWwiseResourceCookerModule
{
public:
	UWwiseResourceCooker* GetDefaultCooker() override;

	UWwiseResourceCooker* CreateCookerForPlatform(
		const ITargetPlatform* TargetPlatform,
		const FWwiseSharedPlatformId& InPlatform,
		EWwiseExportDebugNameRule InExportDebugNameRule = EWwiseExportDebugNameRule::Release) override;
	void DestroyCookerForPlatform(const ITargetPlatform* TargetPlatform) override;
	UWwiseResourceCooker* GetCookerForPlatform(const ITargetPlatform* TargetPlatform) override;

	void DestroyAllCookerPlatforms() override;

protected:
	TMap<const ITargetPlatform*, UWwiseResourceCooker*> CookingPlatforms;

	void OnModifyCook(TConstArrayView<const ITargetPlatform*> InTargetPlatforms, TArray<FName>& InOutPackagesToCook, TArray<FName>& InOutPackagesToNeverCook) override;
};
