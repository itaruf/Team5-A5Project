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

enum class EWwiseMetadataPluginLibType : uint32
{
	Source,
	Effect,
	AudioDevice,
	Metadata,
	Unknown = (uint32)-1
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataPluginLibAttributes : public FWwiseMetadataLoadable
{
	FString LibName;
	uint32 LibId;
	EWwiseMetadataPluginLibType Type;
	FString DLL;
	FString StaticLib;

	FWwiseMetadataPluginLibAttributes(FWwiseMetadataLoader& Loader);

private:
	static EWwiseMetadataPluginLibType TypeFromString(const FString& TypeString);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataPluginLib : public FWwiseMetadataPluginLibAttributes
{
	FWwiseMetadataPluginLib(FWwiseMetadataLoader& Loader);
};
