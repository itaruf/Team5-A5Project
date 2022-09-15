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

#include "Wwise/Metadata/WwiseMetadataLoadable.h"
#include "Wwise/Metadata/WwiseMetadataPlugin.h"

struct WWISEPROJECTDATABASE_API FWwiseMetadataPluginReferenceGroup : public FWwiseMetadataLoadable
{
	TArray<FWwiseMetadataPluginReference> Custom;
	TArray<FWwiseMetadataPluginReference> Sharesets;
	TArray<FWwiseMetadataPluginReference> AudioDevices;

	FWwiseMetadataPluginReferenceGroup(FWwiseMetadataLoader& Loader);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataPluginGroup : public FWwiseMetadataLoadable
{
	TArray<FWwiseMetadataPlugin> Custom;
	TArray<FWwiseMetadataPlugin> Sharesets;
	TArray<FWwiseMetadataPlugin> AudioDevices;

	FWwiseMetadataPluginGroup(FWwiseMetadataLoader& Loader);
};
