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

#include "WwiseMediaCookedData.generated.h"

USTRUCT(BlueprintType)
struct WWISEFILEHANDLER_API FWwiseMediaCookedData
{
	GENERATED_BODY()

	/**
	 * @brief Short ID for the Media
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 MediaId;

	/**
	 * @brief Path name relative to the platform's root.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FString MediaPathName;

	/**
	 * @brief How many bytes need to be retrieved at load-time. Only set if streaming.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 PrefetchSize;

	/**
	 * @brief Alignment required to load the asset on device. Can be 0 if no particular requirements.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 MemoryAlignment;

	/**
	 * @brief True if the asset needs to be loaded in a special memory zone on the device.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	bool bDeviceMemory;

	/**
	 * @brief True if the asset should not be fully loaded in memory at load time.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	bool bStreaming;

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FString DebugName;

	FWwiseMediaCookedData();

	friend uint32 GetTypeHash(const FWwiseMediaCookedData& InCookedData)
	{
		return HashCombine(::GetTypeHash(InCookedData.MediaId), ::GetTypeHash(InCookedData.MediaPathName));
	}
	friend bool operator==(const FWwiseMediaCookedData& InLhs, const FWwiseMediaCookedData& InRhs)
	{
		return InLhs.MediaId == InRhs.MediaId && InLhs.MediaPathName == InRhs.MediaPathName;
	}

	void Serialize(FArchive& Ar);
};
