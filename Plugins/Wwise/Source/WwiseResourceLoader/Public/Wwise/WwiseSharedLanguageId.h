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

#include "Wwise/WwiseLanguageId.h"
#include "Wwise/CookedData/WwiseLanguageCookedData.h"
#include "WwiseSharedLanguageId.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseSharedLanguageId
{
	GENERATED_BODY()

	static const FWwiseSharedLanguageId Sfx;

	TSharedPtr<FWwiseLanguageId> Language;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	EWwiseLanguageRequirement LanguageRequirement;

	FWwiseSharedLanguageId();
	FWwiseSharedLanguageId(int32 InLanguageId, const FString& InLanguageName, EWwiseLanguageRequirement InLanguageRequirement);
	FWwiseSharedLanguageId(const FWwiseLanguageId& InLanguageId, EWwiseLanguageRequirement InLanguageRequirement);
	~FWwiseSharedLanguageId();

	int32 GetLanguageId() const
	{
		return Language->LanguageId;
	}

	const FString& GetLanguageName() const
	{
		return Language->LanguageName;
	}

	bool operator==(const FWwiseSharedLanguageId& Rhs) const
	{
		return (!Language.IsValid() && !Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language == *Rhs.Language);
	}

	bool operator!=(const FWwiseSharedLanguageId& Rhs) const
	{
		return (Language.IsValid() != Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language != *Rhs.Language);
	}

	bool operator>=(const FWwiseSharedLanguageId& Rhs) const
	{
		return (!Language.IsValid() && !Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language >= *Rhs.Language);
	}

	bool operator>(const FWwiseSharedLanguageId& Rhs) const
	{
		return (Language.IsValid() && !Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language > *Rhs.Language);
	}

	bool operator<=(const FWwiseSharedLanguageId& Rhs) const
	{
		return (!Language.IsValid() && !Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language <= *Rhs.Language);
	}

	bool operator<(const FWwiseSharedLanguageId& Rhs) const
	{
		return (!Language.IsValid() && Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language < *Rhs.Language);
	}

	friend uint32 GetTypeHash(const FWwiseSharedLanguageId& Id)
	{
		return ::GetTypeHash(Id.Language->LanguageId);
	}
};
