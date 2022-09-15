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

#include "Wwise/Metadata/WwiseMetadataProjectInfo.h"

#include "Wwise/Metadata/WwiseMetadataLanguage.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Metadata/WwiseMetadataPlatform.h"

FWwiseMetadataProjectInfo::FWwiseMetadataProjectInfo(FWwiseMetadataLoader& Loader) :
	Project(Loader.GetObject<FWwiseMetadataProject>(this, TEXT("Project"))),
	CacheRoot(Loader.GetString(this, TEXT("CacheRoot"))),
	Platforms(Loader.GetArray<FWwiseMetadataPlatformReference>(this, TEXT("Platforms"))),
	Languages(Loader.GetArray<FWwiseMetadataLanguage>(this, TEXT("Languages"))),
	FileHash(Loader.GetGuid(this, TEXT("FileHash")))
{
	Loader.LogParsed(TEXT("ProjectInfo"));
}
