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

#include "Wwise/CookedData/WwiseGroupValueCookedData.h"

#include "WwiseSharedGroupValueKey.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseSharedGroupValueKey
{
	GENERATED_BODY()

	TSharedRef<FWwiseGroupValueCookedData> GroupValueCookedData;

	FWwiseSharedGroupValueKey() :
		GroupValueCookedData(new FWwiseGroupValueCookedData)
	{}

	FWwiseSharedGroupValueKey(const FWwiseGroupValueCookedData& InGroupValueCookedData) :
		GroupValueCookedData(new FWwiseGroupValueCookedData(InGroupValueCookedData))
	{}

	bool operator==(const FWwiseSharedGroupValueKey& Rhs) const
	{
		return *GroupValueCookedData == *Rhs.GroupValueCookedData;
	}

	bool operator!=(const FWwiseSharedGroupValueKey& Rhs) const
	{
		return *GroupValueCookedData != *Rhs.GroupValueCookedData;
	}

	bool operator>=(const FWwiseSharedGroupValueKey& Rhs) const
	{
		return *GroupValueCookedData >= *Rhs.GroupValueCookedData;
	}

	bool operator>(const FWwiseSharedGroupValueKey& Rhs) const
	{
		return *GroupValueCookedData > *Rhs.GroupValueCookedData;
	}

	bool operator<=(const FWwiseSharedGroupValueKey& Rhs) const
	{
		return *GroupValueCookedData <= *Rhs.GroupValueCookedData;
	}

	bool operator<(const FWwiseSharedGroupValueKey& Rhs) const
	{
		return *GroupValueCookedData < *Rhs.GroupValueCookedData;
	}
};

inline uint32 GetTypeHash(const FWwiseSharedGroupValueKey& Id)
{
	return GetTypeHash(*Id.GroupValueCookedData);
}
