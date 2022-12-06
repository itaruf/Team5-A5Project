/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2022 Audiokinetic Inc.
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
	CheckWwiseObjectInfo();
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

AkUInt32 UAkAudioType::GetShortID() const
{
	UE_LOG(LogAkAudio, Error, TEXT("Trying to GetShortID without an overridden Short ID for %s."), *GetName());
	return AK_INVALID_UNIQUE_ID;
}

#if WITH_EDITORONLY_DATA
void UAkAudioType::MigrateWwiseObjectInfo()
{
	if (FWwiseObjectInfo* Info = GetInfoMutable())
	{
		Info->WwiseName = FName(GetName());

		if ( ID_DEPRECATED.IsValid())
		{
			Info->WwiseGuid = ID_DEPRECATED;
		}

		if (ShortID_DEPRECATED != 0)
		{
			Info->WwiseShortId = ShortID_DEPRECATED;
		}
		else
		{
			Info->WwiseShortId = FAkAudioDevice::GetShortIDFromString(GetName());
		}
	}
}

void UAkAudioType::CheckWwiseObjectInfo()
{
	if (FWwiseObjectInfo* WwiseInfo = GetInfoMutable())
	{
		if (!WwiseInfo->WwiseGuid.IsValid() || WwiseInfo->WwiseShortId == 0 || WwiseInfo->WwiseName.ToString().IsEmpty())
		{
			UE_LOG(LogAkAudio, Log, TEXT("CheckWwiseObjectInfo: Wwise Asset %s has empty WwiseObjectInfo fields. WwiseName: '%s' - WwiseShortId: '%d' - WwiseGuid: '%s'"),
				*GetName(), *WwiseInfo->WwiseName.ToString(), WwiseInfo->WwiseShortId, *WwiseInfo->WwiseGuid.ToString());
		}
	}
}

FWwiseObjectInfo* UAkAudioType::GetInfoMutable()
{
	UE_LOG(LogAkAudio, Error, TEXT("GetInfoMutable not implemented"));
	check(false);
	return nullptr;
}

void UAkAudioType::ValidateShortID(FWwiseObjectInfo& WwiseInfo) const
{
	if (WwiseInfo.WwiseShortId == AK_INVALID_UNIQUE_ID)
	{
		if (!WwiseInfo.WwiseName.ToString().IsEmpty())
		{
			WwiseInfo.WwiseShortId = FAkAudioDevice::GetShortIDFromString(WwiseInfo.WwiseName.ToString());
		}
		else
		{
			WwiseInfo.WwiseShortId = FAkAudioDevice::GetShortIDFromString(GetName());
			UE_LOG(LogAkAudio, Warning, TEXT("UAkAudioType::ValidateShortID : Using ShortID based on asset name for %s."), *GetName());
		}
	}
}

#if WITH_EDITOR
void UAkAudioType::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FWwiseObjectInfo* AudioTypeInfo = GetInfoMutable();

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
	{
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseName))
		{
			AudioTypeInfo->WwiseGuid = {};
			AudioTypeInfo->WwiseShortId = 0;
		}
		else if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FWwiseEventInfo, WwiseShortId))
		{
			AudioTypeInfo->WwiseGuid= {};
			AudioTypeInfo->WwiseName = "";
		}
		// The first check should be sufficient, but the property's FName for GUIDs is A/B/C/D,
		// depending on which part of the GUID was modified
		else if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FWwiseEventInfo, WwiseGuid) ||
			PropertyChangedEvent.Property->Owner.GetFName() == "Guid")
		{
			AudioTypeInfo->WwiseName = "";
			AudioTypeInfo->WwiseShortId = {};
		}
		else if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAkAudioType, bAutoLoad))
		{
			if(!bAutoLoad)
			{
				UnloadData();
			}
			// Else not necessary, we will load the data just before exiting the function
		}
		FillInfo();
	}

	if (bAutoLoad)
	{
		LoadData();
	}
}

void UAkAudioType::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	UObject::GetAssetRegistryTags(OutTags);

	auto WwiseInfo = GetInfo();
	if (WwiseInfo.WwiseGuid.IsValid())
	{
		//This seems to be more reliable than putting the AssetRegistrySearchable tag on FWwiseObjectInfo::WwiseGuid
		OutTags.Add(FAssetRegistryTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseGuid), WwiseInfo.WwiseGuid.ToString(), FAssetRegistryTag::ETagType::TT_Hidden));
	}
}
#endif

void UAkAudioType::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	UObject::BeginCacheForCookedPlatformData(TargetPlatform);
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
	auto PlatformID = UAkPlatformInfo::GetSharedPlatformInfo(TargetPlatform->IniPlatformName());
	FWwiseResourceCooker::CreateForPlatform(TargetPlatform, PlatformID, EWwiseExportDebugNameRule::Name);
}

#endif
