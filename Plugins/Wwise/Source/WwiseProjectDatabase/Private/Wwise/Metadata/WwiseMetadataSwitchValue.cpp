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

#include "Wwise/Metadata/WwiseMetadataSwitchValue.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataSwitchValueAttributes::FWwiseMetadataSwitchValueAttributes()
{
}

FWwiseMetadataSwitchValueAttributes::FWwiseMetadataSwitchValueAttributes(FWwiseMetadataLoader& Loader) :
	GroupType(GroupTypeFromString(Loader.GetString(this, TEXT("GroupType")))),
	GroupId(Loader.GetUint32(this, TEXT("GroupId"))),
	Id(Loader.GetUint32(this, TEXT("Id"))),
	GUID(Loader.GetGuid(this, TEXT("GUID"))),
	bDefault(Loader.GetBool(this, TEXT("Default"), EWwiseRequiredMetadata::Optional))
{
	Loader.LogParsed(TEXT("SwitchValueAttributes"));
}

EWwiseMetadataSwitchValueGroupType FWwiseMetadataSwitchValueAttributes::GroupTypeFromString(const FString& TypeString)
{
	if (TypeString.Equals("Switch"))
	{
		return EWwiseMetadataSwitchValueGroupType::Switch;
	}
	else if (TypeString.Equals("State"))
	{
		return EWwiseMetadataSwitchValueGroupType::State;
	}
	UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("FWwiseMetadataSwitchValueAttributes: Unknown GroupType: %s"), *TypeString);
	return EWwiseMetadataSwitchValueGroupType::Unknown;
}

FWwiseMetadataSwitchValue::FWwiseMetadataSwitchValue()
{
}

FWwiseMetadataSwitchValue::FWwiseMetadataSwitchValue(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataSwitchValueAttributes(Loader)
{
	Loader.LogParsed(TEXT("SwitchValue"));
}
