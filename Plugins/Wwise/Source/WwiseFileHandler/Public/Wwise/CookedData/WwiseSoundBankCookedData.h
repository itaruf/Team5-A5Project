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

#include "AkInclude.h"

#include "WwiseSoundBankCookedData.generated.h"

UENUM(BlueprintType)
enum class EWwiseSoundBankType : uint8
{
	User, // = AKCODECID_BANK,
	Event = AKCODECID_BANK_EVENT,
	Bus = AKCODECID_BANK_BUS
};

/**
 * @brief Required data to load a SoundBank
*/
USTRUCT(BlueprintType)
struct WWISEFILEHANDLER_API FWwiseSoundBankCookedData
{
	GENERATED_BODY()

	/**
	 * @brief Short ID for the SoundBank.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 SoundBankId;

	/**
	 * @brief Path name relative to the platform's root.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FString SoundBankPathName;

	/**
	 * @brief Alignment required to load the SoundBank on device. Can be 0 if no particular requirements.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 MemoryAlignment;

	/**
	 * @brief True if the SoundBank needs to be loaded in a special memory zone on the device.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	bool bDeviceMemory;

	/**
	 * @brief True if the SoundBank contains media or media parts. False means a data-only SoundBank.
	 *
	 * Useful to load the SoundBank as a copy instead of keeping it Memory-mapped, as the SoundEngine will decode
	 * data from the SoundBank, and has no use for the file itself.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	bool bContainsMedia;

	/**
	 * @brief User-created SoundBank, Event Auto-defined SoundBank, or Bus Auto-defined SoundBank.
	 *
	 * Useful for loading by file name.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	EWwiseSoundBankType SoundBankType;

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FString DebugName;

	friend uint32 GetTypeHash(const FWwiseSoundBankCookedData& InCookedData)
	{
		return HashCombine(::GetTypeHash(InCookedData.SoundBankId), ::GetTypeHash(InCookedData.SoundBankPathName));
	}

	friend bool operator==(const FWwiseSoundBankCookedData& InLhs, const FWwiseSoundBankCookedData& InRhs)
	{
		return InLhs.SoundBankId == InRhs.SoundBankId && InLhs.SoundBankPathName == InRhs.SoundBankPathName;
	}

	FWwiseSoundBankCookedData();

	void Serialize(FArchive& Ar);
};
