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
#include "Wwise/Info/WwiseBasicInfo.h"
#include "WwiseGroupValueInfo.generated.h"

USTRUCT(BlueprintType, Meta = (Category = "Wwise", DisplayName = "GroupValue Info", HasNativeMake = "WwiseResourceLoader.WwiseGroupValueInfoLibrary.MakeStruct", HasNativeBreak = "WwiseResourceLoader.WwiseGroupValueInfoLibrary.BreakStruct"))
struct WWISERESOURCELOADER_API FWwiseGroupValueInfo: public FWwiseBasicInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnyWhere, Category = "Info")
	uint32 GroupShortId;
	
	FWwiseGroupValueInfo() :
		FWwiseBasicInfo(),
		GroupShortId(0)
	{}

	FWwiseGroupValueInfo(const FWwiseGroupValueInfo& InGroupValueInfo) :
		FWwiseBasicInfo(InGroupValueInfo),
		GroupShortId(InGroupValueInfo.GroupShortId)
	{}

	FWwiseGroupValueInfo(const FGuid& InAssetGuid, uint32 InGroupShortId, uint32 InAssetShortId, const FString& InAssetName) :
		FWwiseBasicInfo(InAssetGuid, InGroupShortId, InAssetName),
		GroupShortId(InGroupShortId)
	{}

	FWwiseGroupValueInfo(uint32 InGroupShortId, uint32 InAssetShortId, const FString& InAssetName) :
		FWwiseBasicInfo(InAssetShortId, InAssetName),
		GroupShortId(InGroupShortId)
	{}

	bool operator==(const FWwiseGroupValueInfo& Rhs) const
	{
		return (!AssetGuid.IsValid() || !Rhs.AssetGuid.IsValid() || AssetGuid == Rhs.AssetGuid) &&
			((GroupShortId == 0 && AssetShortId == 0) || (Rhs.GroupShortId == 0 && Rhs.AssetShortId == 0) || (GroupShortId == Rhs.GroupShortId && AssetShortId == Rhs.AssetShortId)) &&
			(AssetName.IsEmpty() || Rhs.AssetName.IsEmpty() || AssetName == Rhs.AssetName);
	}

	bool operator!=(const FWwiseGroupValueInfo& Rhs) const
	{
		return !operator==(Rhs);
	}
};

inline uint32 GetTypeHash(const FWwiseGroupValueInfo& InValue)
{
	return HashCombine(HashCombine(HashCombine(
		GetTypeHash(InValue.AssetGuid),
		GetTypeHash(InValue.GroupShortId)),
		GetTypeHash(InValue.AssetShortId)),
		GetTypeHash(InValue.AssetName));
}
