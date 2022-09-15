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

#include "Wwise/Metadata/WwiseMetadataExternalSource.h"
#include "Wwise/Metadata/WwiseMetadataSwitchValue.h"

struct WWISEPROJECTDATABASE_API FWwiseMetadataSwitchContainer : public FWwiseMetadataLoadable
{
	FWwiseMetadataSwitchValue SwitchValue;
	TArray<FWwiseMetadataMediaReference> MediaRefs;
	TArray<FWwiseMetadataExternalSourceReference> ExternalSourceRefs;
	FWwiseMetadataPluginReferenceGroup* PluginRefs;
	TArray<FWwiseMetadataSwitchContainer> Children;

	FWwiseMetadataSwitchContainer(FWwiseMetadataLoader& Loader);
	TSet<FWwiseMetadataMediaReference> GetAllMedia() const;
	TSet<FWwiseMetadataExternalSourceReference> GetAllExternalSources() const;
	TSet<FWwiseMetadataPluginReference> GetAllCustomPlugins() const;
	TSet<FWwiseMetadataPluginReference> GetAllPluginSharesets() const;
	TSet<FWwiseMetadataPluginReference> GetAllAudioDevices() const;
};
