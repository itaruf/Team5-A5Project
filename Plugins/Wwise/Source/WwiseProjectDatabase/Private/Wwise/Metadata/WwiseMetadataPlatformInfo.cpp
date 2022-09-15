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

#include "Wwise/Metadata/WwiseMetadataPlatformInfo.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataPlatformInfo::FWwiseMetadataPlatformInfo(FWwiseMetadataLoader& Loader) :
	Platform(Loader.GetObject<FWwiseMetadataPlatform>(this, TEXT("Platform"))),
	RootPaths(Loader.GetObject<FWwiseMetadataRootPaths>(this, TEXT("RootPaths"))),
	DefaultAlign(Loader.GetUint32(this, TEXT("DefaultAlign"))),
	Settings(Loader.GetObject<FWwiseMetadataSettings>(this, TEXT("Settings"))),
	FileHash(Loader.GetGuid(this, TEXT("FileHash")))
{
	Loader.LogParsed(TEXT("PlatformInfo"));
}
