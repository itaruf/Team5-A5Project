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

#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseResourceCookerModule.h"

#include "WwiseResourceCooker.generated.h"

struct FWwiseSoundBankCookedData;
struct FWwiseGameParameterCookedData;
struct FWwiseTriggerCookedData;
struct FWwiseAcousticTextureCookedData;
class UWwiseCookingCache;

UCLASS(Abstract)
class WWISERESOURCECOOKER_API UWwiseResourceCooker : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	using WriteAdditionalFileFunction = TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)>;

	static UWwiseResourceCooker* GetDefault()
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			return Module->GetDefaultCooker();
		}
		return nullptr;
	}
	static UWwiseResourceCooker* CreateForPlatform(
		const ITargetPlatform* TargetPlatform,
		const FWwiseSharedPlatformId& InPlatform,
		EWwiseExportDebugNameRule InExportDebugNameRule = EWwiseExportDebugNameRule::Release)
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			return Module->CreateCookerForPlatform(TargetPlatform, InPlatform, InExportDebugNameRule);
		}
		return nullptr;
	}

	static void DestroyForPlatform(const ITargetPlatform* TargetPlatform)
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			Module->DestroyCookerForPlatform(TargetPlatform);
		}
	}

	static UWwiseResourceCooker* GetForPlatform(const ITargetPlatform* TargetPlatform)
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			return Module->GetCookerForPlatform(TargetPlatform);
		}
		return nullptr;
	}

	static UWwiseResourceCooker* GetForArchive(const FArchive& InArchive)
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			return Module->GetCookerForArchive(InArchive);
		}
		return nullptr;
	}

	void CookAuxBus(const FWwiseAssetInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookEvent(const FWwiseEventInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookExternalSource(uint32 InCookie, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookInitBank(const FWwiseAssetInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookMedia(const FWwiseAssetInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookShareset(const FWwiseAssetInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookSoundBank(const FWwiseAssetInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);

	bool PrepareCookedData(FWwiseAcousticTextureCookedData& OutCookedData, const FWwiseAssetInfo& InInfo);
	bool PrepareCookedData(FWwiseLocalizedAuxBusCookedData& OutCookedData, const FWwiseAssetInfo& InInfo);
	bool PrepareCookedData(FWwiseLocalizedEventCookedData& OutCookedData, const FWwiseEventInfo& InInfo);
	bool PrepareCookedData(FWwiseExternalSourceCookedData& OutCookedData, uint32 InCookie);
	bool PrepareCookedData(FWwiseGameParameterCookedData& OutCookedData, const FWwiseAssetInfo& InInfo);
	bool PrepareCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo, EWwiseGroupType InGroupType);
	bool PrepareCookedData(FWwiseInitBankCookedData& OutCookedData, const FWwiseAssetInfo& InInfo = FWwiseAssetInfo::DefaultInitBank);
	bool PrepareCookedData(FWwiseMediaCookedData& OutCookedData, const FWwiseAssetInfo& InInfo);
	bool PrepareCookedData(FWwiseLocalizedSharesetCookedData& OutCookedData, const FWwiseAssetInfo& InInfo);
	bool PrepareCookedData(FWwiseLocalizedSoundBankCookedData& OutCookedData, const FWwiseAssetInfo& InInfo);
	bool PrepareCookedData(FWwiseTriggerCookedData& OutCookedData, const FWwiseAssetInfo& InInfo);

	virtual UWwiseProjectDatabase* GetProjectDatabase() { return nullptr; }
	virtual const UWwiseProjectDatabase* GetProjectDatabase() const { return nullptr; }

	virtual void PrepareResourceCookerForPlatform(UWwiseProjectDatabase* InProjectDatabaseOverride, EWwiseExportDebugNameRule InExportDebugNameRule) {}

	virtual void SetSandboxRootPath(const TCHAR* InPackageFilename);
	FString GetSandboxRootPath() const {return SandboxRootPath;}

	UWwiseResourceLoader* GetResourceLoader();
	const UWwiseResourceLoader* GetResourceLoader() const;

	UFUNCTION()
	FWwiseSharedLanguageId GetCurrentLanguage() const;

	UFUNCTION()
	FWwiseSharedPlatformId GetCurrentPlatform() const;

	// Low-level operations

	virtual UWwiseCookingCache* GetCookingCache() { return nullptr; }

	void CookLocalizedAuxBusToSandbox(const FWwiseLocalizedAuxBusCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookLocalizedSoundBankToSandbox(const FWwiseLocalizedSoundBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookLocalizedEventToSandbox(const FWwiseLocalizedEventCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookLocalizedSharesetToSandbox(const FWwiseLocalizedSharesetCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile);

	virtual void CookAuxBusToSandbox(const FWwiseAuxBusCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookEventToSandbox(const FWwiseEventCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookExternalSourceToSandbox(const FWwiseExternalSourceCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookInitBankToSandbox(const FWwiseInitBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookMediaToSandbox(const FWwiseMediaCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookSharesetToSandbox(const FWwiseSharesetCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookSoundBankToSandbox(const FWwiseSoundBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}

	virtual void CookFileToSandbox(const FString& InInputPathName, const FString& InOutputPathName, WriteAdditionalFileFunction WriteAdditionalFile, bool bInStageRelativeToContent = false) {}

	virtual bool GetAcousticTextureCookedData(FWwiseAcousticTextureCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const { return false; }
	virtual bool GetAuxBusCookedData(FWwiseLocalizedAuxBusCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const { return false; }
	virtual bool GetEventCookedData(FWwiseLocalizedEventCookedData& OutCookedData, const FWwiseEventInfo& InInfo) const { return false; }
	virtual bool GetExternalSourceCookedData(FWwiseExternalSourceCookedData& OutCookedData, uint32 InCookie) const { return false; }
	virtual bool GetGameParameterCookedData(FWwiseGameParameterCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const { return false; }
	virtual bool GetInitBankCookedData(FWwiseInitBankCookedData& OutCookedData, const FWwiseAssetInfo& InInfo = FWwiseAssetInfo::DefaultInitBank) const { return false; }
	virtual bool GetMediaCookedData(FWwiseMediaCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const { return false; }
	virtual bool GetSharesetCookedData(FWwiseLocalizedSharesetCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const { return false; }
	virtual bool GetSoundBankCookedData(FWwiseLocalizedSoundBankCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const { return false; }
	virtual bool GetStateCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo) const { return false; }
	virtual bool GetSwitchCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo) const { return false; }
	virtual bool GetTriggerCookedData(FWwiseTriggerCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const { return false; }

protected:
	UPROPERTY()
	FString SandboxRootPath;
};