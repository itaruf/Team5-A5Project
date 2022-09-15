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

#include "WwiseLanguageId.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseLanguageId
{
	GENERATED_BODY()

	static const FWwiseLanguageId Sfx;

	FWwiseLanguageId() :
		LanguageId(0),
		LanguageName()
	{}
	FWwiseLanguageId(int32 InLanguageId, const FString& InLanguageName) :
		LanguageId(InLanguageId),
		LanguageName(InLanguageName)
	{
		check(!LanguageName.IsEmpty());
	}

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 LanguageId;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FString LanguageName;

	bool operator==(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId == Rhs.LanguageId;
	}

	bool operator!=(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId != Rhs.LanguageId;
	}

	bool operator>=(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId >= Rhs.LanguageId;
	}

	bool operator>(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId > Rhs.LanguageId;
	}

	bool operator<=(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId <= Rhs.LanguageId;
	}

	bool operator<(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId < Rhs.LanguageId;
	}

	friend uint32 GetTypeHash(const FWwiseLanguageId& Id)
	{
		return ::GetTypeHash(Id.LanguageId);
	}
};
