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

#include "Wwise/WwiseResourceCooker.h"
#include "WwiseResourceCookerImpl.generated.h"

UCLASS()
class WWISERESOURCECOOKER_API UWwiseResourceCookerImpl : public UWwiseResourceCooker
{
	GENERATED_BODY()

public:
	UWwiseResourceCookerImpl();
	~UWwiseResourceCookerImpl() override;

	UPROPERTY()
	EWwiseExportDebugNameRule ExportDebugNameRule;

	UWwiseProjectDatabase* GetProjectDatabase() override;
	const UWwiseProjectDatabase* GetProjectDatabase() const override;

	void PrepareResourceCookerForPlatform(UWwiseProjectDatabase* InProjectDatabaseOverride, EWwiseExportDebugNameRule InExportDebugNameRule) override;

protected:
	UPROPERTY()
	UWwiseCookingCache* CookingCache;

	UPROPERTY()
	UWwiseProjectDatabase* ProjectDatabaseOverride;

	UWwiseCookingCache* GetCookingCache() override { return CookingCache; }

	void CookAuxBusToSandbox(const FWwiseAuxBusCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookEventToSandbox(const FWwiseEventCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookExternalSourceToSandbox(const FWwiseExternalSourceCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookInitBankToSandbox(const FWwiseInitBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookMediaToSandbox(const FWwiseMediaCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookSharesetToSandbox(const FWwiseSharesetCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookSoundBankToSandbox(const FWwiseSoundBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;

	void CookFileToSandbox(const FString& InInputPathName, const FString& InOutputPathName, WriteAdditionalFileFunction WriteAdditionalFile, bool bInStageRelativeToContent = false) override;

	bool GetAcousticTextureCookedData(FWwiseAcousticTextureCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const override;
	bool GetAuxBusCookedData(FWwiseLocalizedAuxBusCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const override;
	bool GetEventCookedData(FWwiseLocalizedEventCookedData& OutCookedData, const FWwiseEventInfo& InInfo) const override;
	bool GetExternalSourceCookedData(FWwiseExternalSourceCookedData& OutCookedData, uint32 InCookie) const override;
	bool GetGameParameterCookedData(FWwiseGameParameterCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const override;
	bool GetInitBankCookedData(FWwiseInitBankCookedData& OutCookedData, const FWwiseAssetInfo& InInfo = FWwiseAssetInfo::DefaultInitBank) const override;
	bool GetMediaCookedData(FWwiseMediaCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const override;
	bool GetSharesetCookedData(FWwiseLocalizedSharesetCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const override;
	bool GetSoundBankCookedData(FWwiseLocalizedSoundBankCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const override;
	bool GetStateCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo) const override;
	bool GetSwitchCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo) const override;
	bool GetTriggerCookedData(FWwiseTriggerCookedData& OutCookedData, const FWwiseAssetInfo& InInfo) const override;

	virtual bool FillSoundBankBaseInfo(FWwiseSoundBankCookedData& OutSoundBankCookedData,
		const FWwiseMetadataPlatformInfo& InPlatformInfo,
		const FWwiseMetadataSoundBank& InSoundBank) const;
	virtual bool FillMediaBaseInfo(FWwiseMediaCookedData& OutMediaCookedData,
		const FWwiseMetadataPlatformInfo& InPlatformInfo,
		const FWwiseMetadataSoundBank& InSoundBank,
		const FWwiseMetadataMediaReference& InMediaReference) const;
	virtual bool FillMediaBaseInfo(FWwiseMediaCookedData& OutMediaCookedData,
		const FWwiseMetadataPlatformInfo& InPlatformInfo,
		const FWwiseMetadataSoundBank& InSoundBank,
		const FWwiseMetadataMedia& InMedia) const;
	virtual bool FillExternalSourceBaseInfo(FWwiseExternalSourceCookedData& OutExternalSourceCookedData,
		const FWwiseMetadataExternalSource& InExternalSource) const;

	virtual bool AddRequirementsForMedia(TSet<FWwiseSoundBankCookedData>& OutSoundBankSet, TSet<FWwiseMediaCookedData>& OutMediaSet,
		const FWwiseRefMedia& InMediaRef, const FWwiseSharedLanguageId& InLanguage,
		const FWwisePlatformDataStructure& InPlatformData) const;
	virtual bool AddRequirementsForExternalSource(TSet<FWwiseExternalSourceCookedData>& OutExternalSourceSet,
		const FWwiseRefExternalSource& InExternalSourceRef) const;
};