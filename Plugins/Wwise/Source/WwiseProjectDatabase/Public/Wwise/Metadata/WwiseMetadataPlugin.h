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

#include "Wwise/Metadata/WwiseMetadataBasicReference.h"
#include "Wwise/Metadata/WwiseMetadataMedia.h"

struct WWISEPROJECTDATABASE_API FWwiseMetadataPluginReference : public FWwiseMetadataLoadable
{
	uint32 Id;

	FWwiseMetadataPluginReference(FWwiseMetadataLoader& Loader);

	friend uint32 GetTypeHash(const FWwiseMetadataPluginReference& Plugin)
	{
		return ::GetTypeHash(Plugin.Id);
	}
	bool operator ==(const FWwiseMetadataPluginReference& Rhs) const
	{
		return Id == Rhs.Id;
	}
	bool operator <(const FWwiseMetadataPluginReference& Rhs) const
	{
		return Id < Rhs.Id;
	}
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataPluginAttributes : public FWwiseMetadataBasicReference
{
	FString LibName;
	uint32 LibId;

	FWwiseMetadataPluginAttributes(FWwiseMetadataLoader& Loader);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataPlugin : public FWwiseMetadataPluginAttributes
{
	TArray<FWwiseMetadataMediaReference> MediaRefs;
	FWwiseMetadataPluginReferenceGroup* PluginRefs;

	FWwiseMetadataPlugin(FWwiseMetadataLoader& Loader);
};
