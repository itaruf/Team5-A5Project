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

#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataPluginReference::FWwiseMetadataPluginReference(FWwiseMetadataLoader& Loader) :
	Id(Loader.GetUint32(this, TEXT("Id")))
{
	Loader.LogParsed(TEXT("PluginReference"), Id);
}

FWwiseMetadataPluginAttributes::FWwiseMetadataPluginAttributes(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBasicReference(Loader),
	LibName(Loader.GetString(this, TEXT("LibName"))),
	LibId(Loader.GetUint32(this, TEXT("LibId")))
{
	Loader.LogParsed(TEXT("PluginAttributes"), Id, *Name);
}

FWwiseMetadataPlugin::FWwiseMetadataPlugin(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataPluginAttributes(Loader),
	MediaRefs(Loader.GetArray<FWwiseMetadataMediaReference>(this, TEXT("MediaRefs"))),
	PluginRefs(Loader.GetObjectPtr<FWwiseMetadataPluginReferenceGroup>(this, TEXT("PluginRefs")))
{
	Loader.LogParsed(TEXT("Plugin"), Id, *Name);
}
