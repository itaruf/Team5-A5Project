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
#include "CoreMinimal.h"
#include "WwiseBasicInfo.h"

#include "WwiseAssetInfo.generated.h"

USTRUCT(BlueprintType, Meta = (Category = "Wwise", DisplayName = "Asset Info", HasNativeMake = "WwiseResourceLoader.WwiseAssetInfoLibrary.MakeStruct", HasNativeBreak = "WwiseResourceLoader.WwiseAssetInfoLibrary.BreakStruct"))
struct WWISERESOURCELOADER_API FWwiseAssetInfo: public FWwiseBasicInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Info")
	uint32 HardCodedSoundBankShortId;

	FWwiseAssetInfo() :
		FWwiseBasicInfo(),
		HardCodedSoundBankShortId(0)
	{}

	FWwiseAssetInfo(const FWwiseAssetInfo& InAssetInfo) :
		FWwiseBasicInfo(InAssetInfo),
		HardCodedSoundBankShortId(InAssetInfo.HardCodedSoundBankShortId)
	{}

	FWwiseAssetInfo(const FGuid& InAssetGuid, uint32 InAssetShortId, const FString& InAssetName, uint32 InHardCodedSoundBankShortId = 0) :
		FWwiseBasicInfo(InAssetGuid, InAssetShortId, InAssetName),
		HardCodedSoundBankShortId(InHardCodedSoundBankShortId)
	{}

	FWwiseAssetInfo(uint32 InAssetShortId, const FString& InAssetName) :
		FWwiseBasicInfo(InAssetShortId, InAssetName),
		HardCodedSoundBankShortId(0)
	{}

	static const FWwiseAssetInfo DefaultInitBank;

	bool operator==(const FWwiseAssetInfo& Rhs) const
	{
		return (FWwiseBasicInfo)*this == (FWwiseBasicInfo)Rhs
			&& HardCodedSoundBankShortId == Rhs.HardCodedSoundBankShortId;
	}

	bool operator!=(const FWwiseAssetInfo& Rhs) const
	{
		return !operator==(Rhs);
	}
};

inline uint32 GetTypeHash(const FWwiseAssetInfo& InValue)
{
	return HashCombine(HashCombine(HashCombine(
		GetTypeHash(InValue.AssetGuid),
		GetTypeHash(InValue.AssetShortId)),
		GetTypeHash(InValue.AssetName)),
		GetTypeHash(InValue.HardCodedSoundBankShortId));
}
