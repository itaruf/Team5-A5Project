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

#include "Wwise/Metadata/WwiseMetadataRootPaths.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataRootPaths::FWwiseMetadataRootPaths()
{
	UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Using default FWwiseMetadataRootPaths"));
}

FWwiseMetadataRootPaths::FWwiseMetadataRootPaths(FWwiseMetadataLoader& Loader) :
	ProjectRoot(Loader.GetString(this, TEXT("ProjectRoot"))),
	SourceFilesRoot(Loader.GetString(this, TEXT("SourceFilesRoot"))),
	SoundBanksRoot(Loader.GetString(this, TEXT("SoundBanksRoot"))),
	ExternalSourcesInputFile(Loader.GetString(this, TEXT("ExternalSourcesInputFile"))),
	ExternalSourcesOutputRoot(Loader.GetString(this, TEXT("ExternalSourcesOutputRoot")))
{
	Loader.LogParsed(TEXT("RootPaths"));
}
