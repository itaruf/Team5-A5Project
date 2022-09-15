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

#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataPluginReferenceGroup::FWwiseMetadataPluginReferenceGroup(FWwiseMetadataLoader& Loader) :
	Custom(Loader.GetArray<FWwiseMetadataPluginReference>(this, TEXT("Custom"))),
	Sharesets(Loader.GetArray<FWwiseMetadataPluginReference>(this, TEXT("Sharesets"))),
	AudioDevices(Loader.GetArray<FWwiseMetadataPluginReference>(this, TEXT("AudioDevices")))
{
	Loader.LogParsed(TEXT("PluginReferenceGroup"));
}

FWwiseMetadataPluginGroup::FWwiseMetadataPluginGroup(FWwiseMetadataLoader& Loader) :
	Custom(Loader.GetArray<FWwiseMetadataPlugin>(this, TEXT("Custom"))),
	Sharesets(Loader.GetArray<FWwiseMetadataPlugin>(this, TEXT("Sharesets"))),
	AudioDevices(Loader.GetArray<FWwiseMetadataPlugin>(this, TEXT("AudioDevices")))
{
	Loader.LogParsed(TEXT("PluginGroup"));
}
