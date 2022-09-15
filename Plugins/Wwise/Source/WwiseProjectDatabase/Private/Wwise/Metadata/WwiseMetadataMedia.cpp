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

#include "Wwise/Metadata/WwiseMetadataMedia.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataMediaReference::FWwiseMetadataMediaReference(FWwiseMetadataLoader& Loader) :
	Id(Loader.GetUint32(this, TEXT("Id")))
{
	Loader.LogParsed(TEXT("MediaReference"), Id);
}

FWwiseMetadataMediaAttributes::FWwiseMetadataMediaAttributes(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataMediaReference(Loader),
	Language(Loader.GetString(this, TEXT("Language"))),
	bStreaming(Loader.GetBool(this, TEXT("Streaming"))),
	Location(LocationFromString(Loader.GetString(this, TEXT("Location")))),
	bUsingReferenceLanguage(Loader.GetBool(this, TEXT("UsingReferenceLanguage"), EWwiseRequiredMetadata::Optional)),
	Align(Loader.GetUint32(this, TEXT("Align"), EWwiseRequiredMetadata::Optional)),
	bDeviceMemory(Loader.GetBool(this, TEXT("DeviceMemory"), EWwiseRequiredMetadata::Optional))
{
	Loader.LogParsed(TEXT("MediaAttributes"), Id);
}

EWwiseMetadataMediaLocation FWwiseMetadataMediaAttributes::LocationFromString(const FString& LocationString)
{
	if (LocationString.Equals(TEXT("Memory")))
	{
		return EWwiseMetadataMediaLocation::Memory;
	}
	else if (LocationString.Equals(TEXT("Loose")))
	{
		return EWwiseMetadataMediaLocation::Loose;
	}
	else if (LocationString.Equals(TEXT("OtherBank")))
	{
		return EWwiseMetadataMediaLocation::OtherBank;
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("FWwiseMetadataMediaAttributes: Unknown Location: %s"), *LocationString);
		return EWwiseMetadataMediaLocation::Unknown;
	}
}

FWwiseMetadataMedia::FWwiseMetadataMedia(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataMediaAttributes(Loader),
	ShortName(Loader.GetString(this, TEXT("ShortName"))),
	Path(Loader.GetString(this, TEXT("Path"), EWwiseRequiredMetadata::Optional)),
	CachePath(Loader.GetString(this, TEXT("CachePath"), EWwiseRequiredMetadata::Optional)),
	PrefetchSize(Loader.GetUint32(this, TEXT("PrefetchSize"), EWwiseRequiredMetadata::Optional))
{
	if (UNLIKELY(Path.IsEmpty() && Location == EWwiseMetadataMediaLocation::Loose))
	{
		Loader.Fail(TEXT("!Path+Location=Loose"));
	}
	else if (UNLIKELY(Path.IsEmpty() && Location == EWwiseMetadataMediaLocation::Memory && bStreaming))
	{
		Loader.Fail(TEXT("!Path+Streaming"));
	}
	else if (UNLIKELY(!Path.IsEmpty() && Location == EWwiseMetadataMediaLocation::Memory && !bStreaming))
	{
		Loader.Fail(TEXT("Path+Memory"));
	}
	Loader.LogParsed(TEXT("Media"), Id);
}
