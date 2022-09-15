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

#include "WwiseBasicInfo.h"
#include "Wwise/CookedData/WwiseEventCookedData.h"

#include "WwiseEventInfo.generated.h"

UENUM(BlueprintType)
enum class EWwiseEventSwitchContainerLoading : uint8
{
	AlwaysLoad UMETA(DisplayName = "Always Load Media"),
	LoadOnReference UMETA(DisplayName = "Load Media Only When Referenced")
};

USTRUCT(BlueprintType, Meta = (Category = "Wwise", DisplayName = "Event Info", HasNativeMake = "WwiseResourceLoader.WwiseEventInfoLibrary.MakeStruct", HasNativeBreak = "WwiseResourceLoader.WwiseEventInfoLibrary.BreakStruct"))
struct WWISERESOURCELOADER_API FWwiseEventInfo: public FWwiseBasicInfo
{
	GENERATED_BODY()

	UPROPERTY()
	EWwiseEventSwitchContainerLoading SwitchContainerLoading;

	UPROPERTY(EditAnywhere, Category = "Info")
	EWwiseEventDestroyOptions DestroyOptions;

	UPROPERTY(EditAnywhere, Category = "Info")
	uint32 HardCodedSoundBankShortId;

	FWwiseEventInfo() :
		FWwiseBasicInfo(),
		SwitchContainerLoading(EWwiseEventSwitchContainerLoading::AlwaysLoad),
		DestroyOptions(EWwiseEventDestroyOptions::StopEventOnDestroy),
		HardCodedSoundBankShortId(0)
	{}

	FWwiseEventInfo(const FWwiseEventInfo& InEventInfo):
		FWwiseBasicInfo(InEventInfo),
		SwitchContainerLoading(InEventInfo.SwitchContainerLoading),
		DestroyOptions(InEventInfo.DestroyOptions),
		HardCodedSoundBankShortId(InEventInfo.HardCodedSoundBankShortId)
	{}

	FWwiseEventInfo(
		const FGuid& InAssetGuid,
		int32 InAssetShortId,
		const FString& InAssetName,
		EWwiseEventSwitchContainerLoading InSwitchContainerLoading = EWwiseEventSwitchContainerLoading::AlwaysLoad,
		EWwiseEventDestroyOptions InDestroyOptions = EWwiseEventDestroyOptions::StopEventOnDestroy,
		uint32 InHardCodedSoundBankShortId = 0) :
		FWwiseBasicInfo(InAssetGuid, InAssetShortId, InAssetName),
		SwitchContainerLoading(InSwitchContainerLoading),
		DestroyOptions(InDestroyOptions),
		HardCodedSoundBankShortId(InHardCodedSoundBankShortId)
	{}

	FWwiseEventInfo(uint32 InAssetShortId, const FString& InAssetName) :
		FWwiseBasicInfo(InAssetShortId, InAssetName),
		SwitchContainerLoading(EWwiseEventSwitchContainerLoading::AlwaysLoad),
		DestroyOptions(EWwiseEventDestroyOptions::StopEventOnDestroy),
		HardCodedSoundBankShortId(0)
	{}

	bool operator==(const FWwiseEventInfo& Rhs) const
	{
		return (FWwiseBasicInfo)*this == (FWwiseBasicInfo)Rhs &&
			HardCodedSoundBankShortId == Rhs.HardCodedSoundBankShortId;
	}

	bool operator!=(const FWwiseEventInfo& Rhs) const
	{
		return !operator==(Rhs);
	}
};

inline uint32 GetTypeHash(const FWwiseEventInfo& InValue)
{
	return HashCombine(HashCombine(HashCombine(
		GetTypeHash(InValue.AssetGuid),
		GetTypeHash(InValue.AssetShortId)),
		GetTypeHash(InValue.AssetName)),
		GetTypeHash(InValue.HardCodedSoundBankShortId));
}