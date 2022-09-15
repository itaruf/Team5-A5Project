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

#include "WwisePlatformId.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwisePlatformId
{
	GENERATED_BODY()

		FWwisePlatformId() :
		PlatformGuid(),
		PlatformName()
#if WITH_EDITORONLY_DATA
		, PathRelativeToGeneratedSoundBanks()
#endif
	{}

	FWwisePlatformId(const FGuid& InPlatformGuid, const FString& InPlatformName) :
		PlatformGuid(InPlatformGuid),
		PlatformName(InPlatformName)
	{}

#if WITH_EDITORONLY_DATA
	FWwisePlatformId(const FGuid& InPlatformGuid, const FString& InPlatformName, const FString& InGeneratedSoundBanksPath) :
		PlatformGuid(InPlatformGuid),
		PlatformName(InPlatformName),
		PathRelativeToGeneratedSoundBanks(InGeneratedSoundBanksPath)
	{}
#endif

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
		FGuid PlatformGuid;

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
		FString PlatformName;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Wwise")
		FString PathRelativeToGeneratedSoundBanks;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Wwise")
		FString ExternalSourceRootPath;
#endif

	bool operator==(const FWwisePlatformId& Rhs) const
	{
		return PlatformGuid == Rhs.PlatformGuid;
	}

	bool operator!=(const FWwisePlatformId& Rhs) const
	{
		return PlatformGuid != Rhs.PlatformGuid;
	}
};
inline uint32 GetTypeHash(const FWwisePlatformId& Id)
{
	return GetTypeHash(Id.PlatformGuid);
}
