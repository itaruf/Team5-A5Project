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

struct WWISEPROJECTDATABASE_API FWwiseMetadataBasicReference : public FWwiseMetadataLoadable
{
	uint32 Id;
	FString Name;
	FString ObjectPath;
	FGuid GUID;

	FWwiseMetadataBasicReference();
	FWwiseMetadataBasicReference(uint32 InId, FString&& InName, FString&& InObjectPath, FGuid&& InGUID) :
		Id(MoveTemp(InId)),
		Name(MoveTemp(InName)),
		ObjectPath(MoveTemp(InObjectPath)),
		GUID(MoveTemp(InGUID))
	{}
	FWwiseMetadataBasicReference(FWwiseMetadataLoader& Loader);

	friend uint32 GetTypeHash(const FWwiseMetadataBasicReference& Ref)
	{
		return ::GetTypeHash(Ref.Id);
	}
	bool operator==(const FWwiseMetadataBasicReference& Rhs) const
	{
		return Id == Rhs.Id;
	}
	bool operator<(const FWwiseMetadataBasicReference& Rhs) const
	{
		return Id < Rhs.Id;
	}
};
