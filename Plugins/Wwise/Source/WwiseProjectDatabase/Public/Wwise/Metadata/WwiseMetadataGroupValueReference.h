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


struct WWISEPROJECTDATABASE_API FWwiseMetadataGroupValueReference : public FWwiseMetadataBasicReference
{
	uint32 GroupId;

	FWwiseMetadataGroupValueReference();
	FWwiseMetadataGroupValueReference(FWwiseMetadataLoader& Loader);

	friend uint32 GetTypeHash(const FWwiseMetadataGroupValueReference& Ref)
	{
		return HashCombine(GetTypeHash((const FWwiseMetadataBasicReference&)Ref), ::GetTypeHash(Ref.Id));
	}
	bool operator==(const FWwiseMetadataGroupValueReference& Rhs) const
	{
		return ((const FWwiseMetadataBasicReference*)this)->operator==(Rhs) && GroupId == Rhs.GroupId;
	}
	bool operator<(const FWwiseMetadataGroupValueReference& Rhs) const
	{
		return ((const FWwiseMetadataBasicReference*)this)->operator<(Rhs) && GroupId < Rhs.GroupId;
	}
};
