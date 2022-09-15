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

#include "Wwise/Ref/WwiseRefMedia.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataMedia.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"

const TCHAR* const FWwiseRefMedia::NAME = TEXT("Media");

const FWwiseMetadataMedia* FWwiseRefMedia::GetMedia() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}

	const auto& Media = SoundBank->Media;
	if (Media.IsValidIndex(MediaIndex))
	{
		return &Media[MediaIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Media index #%zu"), MediaIndex);
		return nullptr;
	}
}

uint32 FWwiseRefMedia::MediaId() const
{
	const auto* Media = GetMedia();
	if (UNLIKELY(!Media))
	{
		return 0;
	}
	return Media->Id;
}

FString FWwiseRefMedia::MediaShortName() const
{
	const auto* Media = GetMedia();
	if (UNLIKELY(!Media))
	{
		return {};
	}
	return Media->ShortName;
}

FString FWwiseRefMedia::MediaPath() const
{
	const auto* Media = GetMedia();
	if (UNLIKELY(!Media))
	{
		return {};
	}
	return Media->Path;
}

uint32 FWwiseRefMedia::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(MediaIndex));
	return Result;
}
