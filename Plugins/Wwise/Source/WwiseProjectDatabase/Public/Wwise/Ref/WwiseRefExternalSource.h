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

#include "Wwise/Ref/WwiseRefSoundBank.h"

class WWISEPROJECTDATABASE_API FWwiseRefExternalSource : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::ExternalSource;
	struct FGlobalIdsMap;

	WwiseRefIndexType ExternalSourceIndex;

	FWwiseRefExternalSource() {}
	FWwiseRefExternalSource(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InExternalSourceIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		ExternalSourceIndex(InExternalSourceIndex)
	{}
	const FWwiseMetadataExternalSource* GetExternalSource() const;

	uint32 ExternalSourceCookie() const;
	FGuid ExternalSourceGuid() const;
	FString ExternalSourceName() const;
	FString ExternalSourceObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefExternalSource& Rhs) const
	{
		return FWwiseRefSoundBank::operator ==(Rhs)
			&& ExternalSourceIndex == Rhs.ExternalSourceIndex;
	}
	bool operator!=(const FWwiseRefExternalSource& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefExternalSource::FGlobalIdsMap
{
	WwiseExternalSourceGlobalIdsMap GlobalIdsMap;
};
