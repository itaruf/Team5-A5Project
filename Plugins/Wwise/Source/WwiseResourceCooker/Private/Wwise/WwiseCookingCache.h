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

#include "Wwise/Info/WwiseEventInfo.h"
#include "Wwise/Info/WwiseGroupValueInfo.h"
#include "Wwise/CookedData/WwiseAcousticTextureCookedData.h"
#include "Wwise/CookedData/WwiseInitBankCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedAuxBusCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedSoundBankCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedEventCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedSharesetCookedData.h"
#include "Wwise/CookedData/WwiseGameParameterCookedData.h"
#include "Wwise/CookedData/WwiseTriggerCookedData.h"

#include "Wwise/Info/WwiseAssetInfo.h"

#include "Wwise/WwiseDatabaseIdentifiers.h"

#include "WwiseCookingCache.generated.h"

class IWwiseExternalSourceManager;

UCLASS(Transient)
class WWISERESOURCECOOKER_API UWwiseCookingCache : public UObject
{
	GENERATED_BODY()

public:
	UWwiseCookingCache() :
		ExternalSourceManager(nullptr)
	{}

	UPROPERTY()
	TMap<FString, FString> StagedFiles;

	UPROPERTY()
	TMap<FWwiseAssetInfo, FWwiseLocalizedAuxBusCookedData> AuxBusCache;

	UPROPERTY()
	TMap<FWwiseAssetInfo, FWwiseLocalizedSoundBankCookedData> SoundBankCache;

	UPROPERTY()
	TMap<FWwiseEventInfo, FWwiseLocalizedEventCookedData> EventCache;

	UPROPERTY()
	TMap<uint32, FWwiseExternalSourceCookedData> ExternalSourceCache;

	UPROPERTY()
	TMap<FWwiseAssetInfo, FWwiseInitBankCookedData> InitBankCache;

	UPROPERTY()
	TMap<FWwiseDatabaseMediaIdKey, FWwiseMediaCookedData> MediaCache;

	UPROPERTY()
	TMap<FWwiseAssetInfo, FWwiseLocalizedSharesetCookedData> SharesetCache;

	UPROPERTY()
	TMap<FWwiseGroupValueInfo, FWwiseGroupValueCookedData> StateCache;

	UPROPERTY()
	TMap<FWwiseGroupValueInfo, FWwiseGroupValueCookedData> SwitchCache;

	UPROPERTY()
	TMap<FWwiseAssetInfo, FWwiseGameParameterCookedData> GameParameterCache;

	UPROPERTY()
	TMap<FWwiseAssetInfo, FWwiseAcousticTextureCookedData> AcousticTextureCache;

	UPROPERTY()
	TMap<FWwiseAssetInfo, FWwiseTriggerCookedData> TriggerCache;

	IWwiseExternalSourceManager* ExternalSourceManager;
};
