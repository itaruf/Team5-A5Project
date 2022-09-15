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

#include "WwiseExternalSourceCookedData.generated.h"

USTRUCT(BlueprintType)
struct WWISEFILEHANDLER_API FWwiseExternalSourceCookedData
{
	GENERATED_BODY()

	/**
	 * @brief User-defined Cookie for the External Source
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 Cookie;

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FString DebugName;

	friend uint32 GetTypeHash(const FWwiseExternalSourceCookedData& InCookedData)
	{
		return ::GetTypeHash(InCookedData.Cookie);
	}
	friend bool operator==(const FWwiseExternalSourceCookedData& InLhs, const FWwiseExternalSourceCookedData& InRhs)
	{
		return InLhs.Cookie == InRhs.Cookie;
	}

	FWwiseExternalSourceCookedData();

	void Serialize(FArchive& Ar);
};
