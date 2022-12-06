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

#pragma once

#include "Wwise/CookedData/WwiseLocalizedAuxBusCookedData.h"
#include "CoreMinimal.h"
#include "Async/Future.h"

struct WWISERESOURCELOADER_API FWwiseLoadedAuxBusInfo
{
	FWwiseLoadedAuxBusInfo(const FWwiseLocalizedAuxBusCookedData& InAuxBus, const FWwiseLanguageCookedData& InLanguage) :
		LocalizedAuxBusCookedData(InAuxBus),
		LanguageRef(InLanguage)
	{}
	FWwiseLoadedAuxBusInfo& operator=(const FWwiseLoadedAuxBusInfo&) = delete;

	const FWwiseLocalizedAuxBusCookedData LocalizedAuxBusCookedData;
	FWwiseLanguageCookedData LanguageRef;

	struct FLoadedData
	{
		FLoadedData() {}
		FLoadedData(const FLoadedData&) = delete;
		FLoadedData& operator=(const FLoadedData&) = delete;

		TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
		TArray<const FWwiseMediaCookedData*> LoadedMedia;

		bool IsLoaded() const
		{
			return LoadedSoundBanks.Num() > 0 || LoadedMedia.Num() > 0;
		}
	} LoadedData;

private:
	friend class TDoubleLinkedList<FWwiseLoadedAuxBusInfo>::TDoubleLinkedListNode;
	FWwiseLoadedAuxBusInfo(const FWwiseLoadedAuxBusInfo& InOriginal) :
		LocalizedAuxBusCookedData(InOriginal.LocalizedAuxBusCookedData),
		LanguageRef(InOriginal.LanguageRef)
	{
	}
};

using FWwiseLoadedAuxBusList = TDoubleLinkedList<FWwiseLoadedAuxBusInfo>;
using FWwiseLoadedAuxBusListNode = FWwiseLoadedAuxBusList::TDoubleLinkedListNode;
using FWwiseLoadedAuxBus = FWwiseLoadedAuxBusListNode*;
using FWwiseLoadedAuxBusPromise = TPromise<FWwiseLoadedAuxBus>;
using FWwiseLoadedAuxBusFuture = TSharedFuture<FWwiseLoadedAuxBus>;
