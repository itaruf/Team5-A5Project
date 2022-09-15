/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#include "AkAudioType.h"

#include "AkCustomVersion.h"
#include "AkAudioDevice.h"
#include "Platforms/AkPlatformInfo.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseResourceCooker.h"
#endif

void UAkAudioType::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	Ar.UsingCustomVersion(FAkCustomVersion::GUID);

#if WITH_EDITORONLY_DATA
	if (ID_DEPRECATED.IsValid() || ShortID_DEPRECATED != 0)
	{
		MigrateIds();
	}

	if (FWwiseBasicInfo* WwiseInfo = GetInfoMutable())
	{
		WwiseGuid = WwiseInfo->AssetGuid;
	}

#endif
	LogSerializationState(Ar);
}

void UAkAudioType::LogSerializationState(const FArchive& Ar)
{
	FString SerializationState = TEXT("");
	if (Ar.IsLoading())
	{
		SerializationState += TEXT("Loading");
	}

	if (Ar.IsSaving())
	{
		SerializationState += TEXT("Saving");
	}

	if (Ar.IsCooking())
	{
		SerializationState += TEXT("Cooking");
	}

	UE_LOG(LogAkAudio, VeryVerbose, TEXT("%s - Serialization - %s"), *GetName(), *SerializationState);

}

#if WITH_EDITORONLY_DATA
void UAkAudioType::MigrateIds()
{
	if (FWwiseBasicInfo* Info = GetInfoMutable())
	{
		Info->AssetGuid = ID_DEPRECATED;
		Info->AssetShortId = ShortID_DEPRECATED;
		Info->AssetName = GetName();
		MarkPackageDirty();
	}
}

FWwiseBasicInfo* UAkAudioType::GetInfoMutable()
{
	UE_LOG(LogAkAudio, Error, TEXT("GetInfoMutable not implemented"));
	check(false);
	return nullptr;
}

void UAkAudioType::ValidateShortID(FWwiseBasicInfo& WwiseInfo) const
{
	if (WwiseInfo.AssetShortId == AK_INVALID_UNIQUE_ID)
	{
		if (!WwiseInfo.AssetName.IsEmpty())
		{
			WwiseInfo.AssetShortId = FAkAudioDevice::GetShortIDFromString(WwiseInfo.AssetName);
		}
		else
		{
			WwiseInfo.AssetShortId = FAkAudioDevice::GetShortIDFromString(GetName());
			UE_LOG(LogAkAudio, Warning, TEXT("UAkAudioType::ValidateShortID : Using ShortID based on asset name for %s."), *GetName());
		}
	}
}

void UAkAudioType::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	UObject::BeginCacheForCookedPlatformData(TargetPlatform);
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
	auto PlatformID = UAkPlatformInfo::GetSharedPlatformInfo(TargetPlatform->PlatformName());
	UWwiseResourceCooker::CreateForPlatform(TargetPlatform, PlatformID, EWwiseExportDebugNameRule::Name);
}

#endif
