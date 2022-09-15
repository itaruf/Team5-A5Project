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

#include "Wwise/Metadata/WwiseMetadataForwardDeclarations.h"

#include "CoreMinimal.h"

class FJsonObject;
class FJsonValue;

struct WWISEPROJECTDATABASE_API FWwiseMetadataLoadable
{
protected:
	TSet<FString> RequestedValues;
	size_t LoadedSize;

	inline FWwiseMetadataLoadable() :
		RequestedValues(),
		LoadedSize(0)
	{}

	inline ~FWwiseMetadataLoadable()
	{
		UnloadLoadedSize();
	}

public:
	void AddRequestedValue(const FString& Type, const FString& Value);
	void CheckRequestedValues(TSharedRef<FJsonObject>& JsonObject);
	void IncLoadedSize(size_t Size);
	void DecLoadedSize(size_t Size);
	void UnloadLoadedSize();
};
