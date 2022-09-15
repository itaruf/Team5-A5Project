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

#include "WwiseBasicInfo.generated.h"

USTRUCT(BlueprintType, Meta = (Category = "Wwise", DisplayName = "Basic Info", HasNativeMake = "WwiseResourceLoader.WwiseBasicInfoLibrary.MakeStruct", HasNativeBreak = "WwiseResourceLoader.WwiseBasicInfoLibrary.BreakStruct"))
struct WWISERESOURCELOADER_API FWwiseBasicInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Info")
	FGuid AssetGuid;

	UPROPERTY(EditAnywhere, Category = "Info")
	uint32 AssetShortId;

	UPROPERTY(EditAnywhere, Category = "Info")
	FString AssetName;

	FWwiseBasicInfo(): AssetShortId(0){}

	FWwiseBasicInfo(const FWwiseBasicInfo& InBasicInfo):
		AssetGuid(InBasicInfo.AssetGuid),
		AssetShortId(InBasicInfo.AssetShortId),
		AssetName(InBasicInfo.AssetName)
	{}

	FWwiseBasicInfo(uint32 InAssetShortId, const FString& InAssetName):
		AssetShortId(InAssetShortId),
		AssetName(InAssetName)
	{}

	FWwiseBasicInfo(
		const FGuid& InAssetGuid, uint32 InAssetShortId, const FString& InAssetName):
		AssetGuid(InAssetGuid),
		AssetShortId(InAssetShortId),
		AssetName(InAssetName)
	{}

	bool operator==(const FWwiseBasicInfo& Rhs) const
	{
		return (!AssetGuid.IsValid() || !Rhs.AssetGuid.IsValid() || AssetGuid == Rhs.AssetGuid) &&
			(AssetShortId == 0 || Rhs.AssetShortId == 0 || AssetShortId == Rhs.AssetShortId) &&
			(AssetName.IsEmpty() || Rhs.AssetName.IsEmpty() || AssetName == Rhs.AssetName);
	}

	bool operator!=(const FWwiseBasicInfo& Rhs) const
	{
		return !operator==(Rhs);
	}
};
