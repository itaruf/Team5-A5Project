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

#include "Wwise/Ref/WwiseRefSoundBanksInfo.h"

#include "Wwise/Metadata/WwiseMetadataRootFile.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefSoundBanksInfo::NAME = TEXT("SoundbanksInfo");

const FWwiseMetadataSoundBanksInfo* FWwiseRefSoundBanksInfo::GetSoundBanksInfo() const
{
	const auto* RootFile = GetRootFile();
	if (!RootFile)
	{
		return nullptr;
	}
	UE_CLOG(!RootFile->SoundBanksInfo, LogWwiseProjectDatabase, Error, TEXT("Could not get SoundBanksInfo"));
	return RootFile->SoundBanksInfo;
}
