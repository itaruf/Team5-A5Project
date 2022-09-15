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

enum class EWwiseMetadataSwitchValueGroupType : uint32
{
	Switch,
	State,
	Unknown = (uint32)-1
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataSwitchValueAttributes : public FWwiseMetadataLoadable
{
	EWwiseMetadataSwitchValueGroupType GroupType;
	uint32 GroupId;
	uint32 Id;
	FGuid GUID;
	bool bDefault;

	FWwiseMetadataSwitchValueAttributes();
	FWwiseMetadataSwitchValueAttributes(FWwiseMetadataLoader& Loader);

private:
	static EWwiseMetadataSwitchValueGroupType GroupTypeFromString(const FString& TypeString);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataSwitchValue : public FWwiseMetadataSwitchValueAttributes
{
	FWwiseMetadataSwitchValue();
	FWwiseMetadataSwitchValue(FWwiseMetadataLoader& Loader);
};
