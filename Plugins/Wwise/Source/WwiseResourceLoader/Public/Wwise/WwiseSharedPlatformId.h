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

#include "Wwise/WwisePlatformId.h"

#include "WwiseSharedPlatformId.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseSharedPlatformId
{
	GENERATED_BODY()

	TSharedRef<FWwisePlatformId> Platform;

	FWwiseSharedPlatformId() :
		Platform(new FWwisePlatformId)
	{}

	FWwiseSharedPlatformId(const FGuid& InPlatformGuid, const FString& InPlatformName) :
		Platform(new FWwisePlatformId(InPlatformGuid, InPlatformName))
	{}

#if WITH_EDITORONLY_DATA
	FWwiseSharedPlatformId(const FGuid& InPlatformGuid, const FString& InPlatformName, const FString& InRelativePath) :
		Platform(new FWwisePlatformId(InPlatformGuid, InPlatformName, InRelativePath))
	{}
#endif

	const FGuid& GetPlatformGuid() const
	{
		return Platform->PlatformGuid;
	}

	const FString& GetPlatformName() const
	{
		return Platform->PlatformName;
	}

	bool operator==(const FWwiseSharedPlatformId& Rhs) const
	{
		return Platform->PlatformGuid == Rhs.Platform->PlatformGuid;
	}

	bool operator!=(const FWwiseSharedPlatformId& Rhs) const
	{
		return Platform->PlatformGuid != Rhs.Platform->PlatformGuid;
	}
};

inline uint32 GetTypeHash(const FWwiseSharedPlatformId& Id)
{
	return GetTypeHash(Id.Platform->PlatformGuid);
}
