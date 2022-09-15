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

#include "Wwise/Ref/WwiseRefProjectInfo.h"

class WWISEPROJECTDATABASE_API FWwiseRefLanguage : public FWwiseRefProjectInfo
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::Language;

	WwiseRefIndexType LanguageIndex;

	FWwiseRefLanguage() :
		LanguageIndex(INDEX_NONE)
	{}
	FWwiseRefLanguage(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InLanguageIndex) :
		FWwiseRefProjectInfo(InRootMediaRef, InJsonFilePath),
		LanguageIndex(InLanguageIndex)
	{}
	const FWwiseMetadataLanguage* GetLanguage() const;

	uint32 LanguageId() const;
	FGuid LanguageGuid() const;
	FString LanguageName() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefLanguage& Rhs) const
	{
		return FWwiseRefProjectInfo::operator==(Rhs)
			&& LanguageIndex == Rhs.LanguageIndex;
	}
	bool operator!=(const FWwiseRefLanguage& Rhs) const { return !operator==(Rhs); }
};
