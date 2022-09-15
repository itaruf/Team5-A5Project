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

#include "Wwise/Metadata/WwiseMetadataBus.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"

FWwiseMetadataBusReference::FWwiseMetadataBusReference(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBasicReference(Loader)
{
	Loader.LogParsed(TEXT("BusReference"), Id, *Name);
}

FWwiseMetadataBus::FWwiseMetadataBus(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBusReference(Loader),
	PluginRefs(Loader.GetObjectPtr<FWwiseMetadataPluginReferenceGroup>(this, TEXT("PluginRefs"))),
	AuxBusRefs(Loader.GetArray<FWwiseMetadataBusReference>(this, TEXT("AuxBusRefs")))
{
	Loader.LogParsed(TEXT("Bus"), Id, *Name);
}
