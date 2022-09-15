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

#include "Wwise/Metadata/WwiseMetadataPluginLib.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataPluginLibAttributes::FWwiseMetadataPluginLibAttributes(FWwiseMetadataLoader& Loader) :
	LibName(Loader.GetString(this, TEXT("LibName"))),
	LibId(Loader.GetUint32(this, TEXT("LibId"))),
	Type(TypeFromString(Loader.GetString(this, TEXT("Type")))),
	DLL(Loader.GetString(this, TEXT("DLL"), EWwiseRequiredMetadata::Optional)),
	StaticLib(Loader.GetString(this, TEXT("StaticLib"), EWwiseRequiredMetadata::Optional))
{
	Loader.LogParsed(TEXT("PluginLibAttributes"), LibId, *LibName);
}

EWwiseMetadataPluginLibType FWwiseMetadataPluginLibAttributes::TypeFromString(const FString& TypeString)
{
	if (TypeString.Equals(TEXT("Effect")))
	{
		return EWwiseMetadataPluginLibType::Effect;
	}
	else if (TypeString.Equals(TEXT("Source")))
	{
		return EWwiseMetadataPluginLibType::Source;
	}
	else if (TypeString.Equals(TEXT("AudioDevice")))
	{
		return EWwiseMetadataPluginLibType::AudioDevice;
	}
	else if (TypeString.Equals(TEXT("Metadata")))
	{
		return EWwiseMetadataPluginLibType::Metadata;
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("FWwiseMetadataPluginLibAttributes: Unknown Type: %s"), *TypeString);
		return EWwiseMetadataPluginLibType::Unknown;
	}
}

FWwiseMetadataPluginLib::FWwiseMetadataPluginLib(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataPluginLibAttributes(Loader)
{
	Loader.LogParsed(TEXT("PluginLib"), LibId, *LibName);
}
