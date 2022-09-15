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

#include "Wwise/Metadata/WwiseMetadataCollections.h"
#include "Wwise/Metadata/WwiseMetadataLoadable.h"


struct WWISEPROJECTDATABASE_API FWwiseMetadataRootFile : public FWwiseMetadataLoadable
{
	FWwiseMetadataPlatformInfo* PlatformInfo;
	FWwiseMetadataPluginInfo* PluginInfo;
	FWwiseMetadataProjectInfo* ProjectInfo;
	FWwiseMetadataSoundBanksInfo* SoundBanksInfo;

	FWwiseMetadataRootFile(FWwiseMetadataLoader& Loader);
	~FWwiseMetadataRootFile();

	static WwiseMetadataSharedRootFilePtr LoadFile(const FString& FilePath);
	static WwiseMetadataSharedRootFilePtr LoadFile(FString&& File, const FString& FilePath);
	static WwiseMetadataFileMap LoadFiles(const TArray<FString>& FilePaths);
};
