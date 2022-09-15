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
#include "Wwise/WwiseSharedPlatformId.h"

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Containers/Map.h"

struct WWISEPROJECTDATABASE_API FWwiseGeneratedFiles
{
	using FileTuple = TTuple<FString, FDateTime>;
	using FileDateTimeMap = TMap<FString, FDateTime>;

	struct WWISEPROJECTDATABASE_API FGeneratedRootFiles
	{
		FileTuple ProjectInfoFile;
		FileTuple WwiseIDsFile;
	};

	struct WWISEPROJECTDATABASE_API FPlatformFiles
	{
		FileTuple PlatformInfoFile;
		FileTuple PluginInfoFile;
		FileTuple SoundbanksInfoFile;

		FileDateTimeMap SoundBankFiles;
		FileDateTimeMap MediaFiles;
		FileDateTimeMap MetadataFiles;
		FileDateTimeMap ExtraFiles;

		TArray<FString> DirectoriesToWatch;
		TArray<FString> LanguageDirectories;
		TArray<FString> AutoSoundBankDirectories;
		FString MediaDirectory;

		bool IsValid() const;

		void Append(FPlatformFiles&& Rhs);
	};

	FGeneratedRootFiles GeneratedRootFiles;
	TMap<FWwiseSharedPlatformId, FPlatformFiles> Platforms;
	WwiseMetadataSharedRootFileConstPtr ProjectInfo;

	bool IsValid() const;
};
