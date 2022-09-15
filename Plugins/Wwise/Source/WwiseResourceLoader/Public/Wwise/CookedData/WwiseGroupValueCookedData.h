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

#include "WwiseGroupValueCookedData.generated.h"

UENUM(BlueprintType)
enum class EWwiseGroupType : uint8
{
	Switch,
	State,
	Unknown = (uint8)-1
};

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseGroupValueCookedData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	EWwiseGroupType Type;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 GroupId;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 Id;

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FString DebugName;

	FWwiseGroupValueCookedData();

	friend bool operator==(const FWwiseGroupValueCookedData& InLhs, const FWwiseGroupValueCookedData& InRhs)
	{
		return InLhs.Type == InRhs.Type
			&& InLhs.GroupId == InRhs.GroupId
			&& InLhs.Id == InRhs.Id;
	}

	friend bool operator!=(const FWwiseGroupValueCookedData& InLhs, const FWwiseGroupValueCookedData& InRhs)
	{
		return !(InLhs == InRhs);
	}

	friend bool operator<(const FWwiseGroupValueCookedData& InLhs, const FWwiseGroupValueCookedData& InRhs)
	{
		return (InLhs.Type < InRhs.Type)
			|| (InLhs.Type == InRhs.Type && InLhs.GroupId < InRhs.GroupId)
			|| (InLhs.Type == InRhs.Type && InLhs.GroupId == InRhs.GroupId && InLhs.Id < InRhs.Id);
	}

	friend bool operator<=(const FWwiseGroupValueCookedData& InLhs, const FWwiseGroupValueCookedData& InRhs)
	{
		return InLhs == InRhs || InLhs < InRhs;
	}

	friend bool operator>(const FWwiseGroupValueCookedData& InLhs, const FWwiseGroupValueCookedData& InRhs)
	{
		return !(InLhs <= InRhs);
	}

	friend bool operator>=(const FWwiseGroupValueCookedData& InLhs, const FWwiseGroupValueCookedData& InRhs)
	{
		return !(InLhs < InRhs);
	}

	void Serialize(FArchive& Ar);
};
inline uint32 GetTypeHash(const FWwiseGroupValueCookedData& InCookedData)
{
	return HashCombine(HashCombine(
		GetTypeHash((uint8)InCookedData.Type),
		GetTypeHash(InCookedData.GroupId)),
		GetTypeHash(InCookedData.Id));
}
