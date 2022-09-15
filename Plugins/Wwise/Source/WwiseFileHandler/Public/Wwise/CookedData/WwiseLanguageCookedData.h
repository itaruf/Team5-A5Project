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

#include "WwiseLanguageCookedData.generated.h"

UENUM(BlueprintType)
enum class EWwiseLanguageRequirement : uint8
{
	IsDefault,
	IsOptional,
	SFX
};

USTRUCT(BlueprintType)
struct WWISEFILEHANDLER_API FWwiseLanguageCookedData
{
	GENERATED_BODY()

	static const FWwiseLanguageCookedData Sfx;

	/**
	 * @brief Short ID for the Language
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 LanguageId;

	/**
	 * @brief Language name as set in Wwise
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FString LanguageName;

	/**
	 * @brief Is this language the default in Wwise
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	EWwiseLanguageRequirement LanguageRequirement;

	FWwiseLanguageCookedData();
	FWwiseLanguageCookedData(int32 LangId, const FString& LangName, EWwiseLanguageRequirement LangRequirement);

	friend uint32 GetTypeHash(const FWwiseLanguageCookedData& InCookedData)
	{
		return HashCombine(::GetTypeHash(InCookedData.LanguageId), ::GetTypeHash(InCookedData.LanguageName));
	}
	friend bool operator==(const FWwiseLanguageCookedData& InLhs, const FWwiseLanguageCookedData& InRhs)
	{
		return InLhs.LanguageId == InRhs.LanguageId && InLhs.LanguageName == InRhs.LanguageName;
	}

	friend bool operator!=(const FWwiseLanguageCookedData& InLhs, const FWwiseLanguageCookedData& InRhs)
	{
		return (InLhs.LanguageId != InRhs.LanguageId) || (InLhs.LanguageName != InRhs.LanguageName);
	}

	void Serialize(FArchive& Ar);

	FString GetLanguageName() const { return LanguageName; }
	int32 GetLanguageId() const { return LanguageId; }
};
