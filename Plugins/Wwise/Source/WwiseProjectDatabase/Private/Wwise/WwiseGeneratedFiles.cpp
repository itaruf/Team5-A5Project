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

#include "Wwise/WwiseGeneratedFiles.h"
#include "Wwise/Metadata//WwiseMetadataRootFile.h"

bool FWwiseGeneratedFiles::FPlatformFiles::IsValid() const
{
	return !PlatformInfoFile.Get<0>().IsEmpty()
		&& !PluginInfoFile.Get<0>().IsEmpty()
		&& (!SoundbanksInfoFile.Get<0>().IsEmpty() || SoundBankFiles.Num() > 0);
}

void FWwiseGeneratedFiles::FPlatformFiles::Append(FPlatformFiles&& Rhs)
{
	SoundBankFiles.Append(MoveTemp(Rhs.SoundBankFiles));
	MediaFiles.Append(MoveTemp(Rhs.MediaFiles));
	MetadataFiles.Append(MoveTemp(Rhs.MetadataFiles));
	ExtraFiles.Append(MoveTemp(Rhs.ExtraFiles));

	DirectoriesToWatch.Append(MoveTemp(Rhs.DirectoriesToWatch));
	LanguageDirectories.Append(MoveTemp(Rhs.LanguageDirectories));
	AutoSoundBankDirectories.Append(MoveTemp(Rhs.AutoSoundBankDirectories));
}

bool FWwiseGeneratedFiles::IsValid() const
{
	return !GeneratedRootFiles.ProjectInfoFile.Get<0>().IsEmpty()
		&& ProjectInfo.IsValid() && ProjectInfo->ProjectInfo;
}
