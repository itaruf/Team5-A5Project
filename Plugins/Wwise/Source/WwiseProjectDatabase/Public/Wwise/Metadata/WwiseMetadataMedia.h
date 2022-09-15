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

struct WWISEPROJECTDATABASE_API FWwiseMetadataMediaReference : public FWwiseMetadataLoadable
{
	uint32 Id;

	FWwiseMetadataMediaReference(FWwiseMetadataLoader& Loader);

	friend uint32 GetTypeHash(const FWwiseMetadataMediaReference& Media)
	{
		return ::GetTypeHash(Media.Id);
	}
	bool operator ==(const FWwiseMetadataMediaReference& Rhs) const
	{
		return Id == Rhs.Id;
	}
	bool operator <(const FWwiseMetadataMediaReference& Rhs) const
	{
		return Id < Rhs.Id;
	}
};

enum class EWwiseMetadataMediaLocation : uint32
{
	Memory,
	Loose,
	OtherBank,

	Unknown = (uint32)-1
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataMediaAttributes : public FWwiseMetadataMediaReference
{
	FString Language;
	bool bStreaming;
	EWwiseMetadataMediaLocation Location;
	bool bUsingReferenceLanguage;
	uint32 Align;
	bool bDeviceMemory;

	FWwiseMetadataMediaAttributes(FWwiseMetadataLoader& Loader);

private:
	static EWwiseMetadataMediaLocation LocationFromString(const FString& LocationString);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataMedia : public FWwiseMetadataMediaAttributes
{
	FString ShortName;
	FString Path;
	FString CachePath;
	uint32 PrefetchSize;

	FWwiseMetadataMedia(FWwiseMetadataLoader& Loader);
};
