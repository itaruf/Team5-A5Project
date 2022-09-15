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

#include "Wwise/Metadata/WwiseMetadataPluginInfo.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataPluginInfoAttributes::FWwiseMetadataPluginInfoAttributes(FWwiseMetadataLoader& Loader) :
	Platform(Loader.GetString(this, TEXT("Platform"))),
	BasePlatform(Loader.GetString(this, TEXT("BasePlatform")))
{
	Loader.LogParsed(TEXT("PluginInfoAttributes"));
}

FWwiseMetadataPluginInfo::FWwiseMetadataPluginInfo(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataPluginInfoAttributes(Loader),
	PluginLibs(Loader.GetArray<FWwiseMetadataPluginLib>(this, TEXT("PluginLibs"))),
	FileHash(Loader.GetGuid(this, TEXT("FileHash")))
{
	Loader.LogParsed(TEXT("PluginInfo"));
}
