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

#include "Wwise/Metadata/WwiseMetadataSoundBanksInfo.h"
#include "Wwise/Metadata/WwiseMetadataRootPaths.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataSoundBanksInfoAttributes::FWwiseMetadataSoundBanksInfoAttributes(FWwiseMetadataLoader& Loader):
	Platform(Loader.GetString(this, TEXT("Platform"))),
	BasePlatform(Loader.GetString(this, TEXT("BasePlatform"))),
	SchemaVersion(Loader.GetUint32(this, TEXT("SchemaVersion"))),
	SoundBankVersion(Loader.GetUint32(this, TEXT("SoundBankVersion")))
{
	Loader.LogParsed(TEXT("SoundBanksInfoAttributes"));
}

FWwiseMetadataSoundBanksInfo::FWwiseMetadataSoundBanksInfo(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataSoundBanksInfoAttributes(Loader),
	RootPaths(Loader.GetObjectPtr<FWwiseMetadataRootPaths>(this, TEXT("RootPaths"))),
	DialogueEvents(Loader.GetArray<FWwiseMetadataDialogueEvent>(this, TEXT("DialogueEvents"))),
	SoundBanks(Loader.GetArray<FWwiseMetadataSoundBank>(this, TEXT("SoundBanks"))),
	FileHash(Loader.GetGuid(this, TEXT("FileHash")))
{
	Loader.LogParsed(TEXT("SoundBanksInfo"));
}

FWwiseMetadataSoundBanksInfo::~FWwiseMetadataSoundBanksInfo()
{
	if (RootPaths)
	{
		delete RootPaths;
		RootPaths = nullptr;
	}
}
