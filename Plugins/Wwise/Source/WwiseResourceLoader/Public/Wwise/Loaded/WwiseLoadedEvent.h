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

#include "Wwise/CookedData/WwiseLocalizedEventCookedData.h"
#include "Wwise/Loaded/WwiseLoadedGroupValue.h"

struct WWISERESOURCELOADER_API FWwiseLoadedEventInfo
{
	FWwiseLoadedEventInfo(const FWwiseLocalizedEventCookedData& InEvent, const FWwiseLanguageCookedData& InLanguage) :
		LocalizedEventCookedData(InEvent),
		LanguageRef(InLanguage)
	{}
	FWwiseLoadedEventInfo& operator=(const FWwiseLoadedEventInfo&) = delete;

	const FWwiseLocalizedEventCookedData LocalizedEventCookedData;
	FWwiseLanguageCookedData LanguageRef;

	struct FLoadedData
	{
		FLoadedData() {}
		FLoadedData(const FLoadedData&) = delete;
		FLoadedData& operator=(const FLoadedData&) = delete;

		TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
		TArray<const FWwiseExternalSourceCookedData*> LoadedExternalSources;
		TArray<const FWwiseMediaCookedData*> LoadedMedia;

		FWwiseLoadedGroupValueList LoadedRequiredGroupValues;
		bool bLoadedSwitchContainerLeaves = false;

		bool IsLoaded() const
		{
			return bLoadedSwitchContainerLeaves || LoadedSoundBanks.Num() > 0 || LoadedExternalSources.Num() > 0 || LoadedMedia.Num() > 0 || LoadedRequiredGroupValues.Num() > 0;
		}
	} LoadedData;

private:
	friend class TDoubleLinkedList<FWwiseLoadedEventInfo>::TDoubleLinkedListNode;
	FWwiseLoadedEventInfo(const FWwiseLoadedEventInfo& InOriginal) :
		LocalizedEventCookedData(InOriginal.LocalizedEventCookedData),
		LanguageRef(InOriginal.LanguageRef)
	{
	}
};

using FWwiseLoadedEventList = TDoubleLinkedList<FWwiseLoadedEventInfo>;
using FWwiseLoadedEventListNode = FWwiseLoadedEventList::TDoubleLinkedListNode;
using FWwiseLoadedEvent = FWwiseLoadedEventListNode*;
using FWwiseLoadedEventPromise = TPromise<FWwiseLoadedEvent>;
using FWwiseLoadedEventFuture = TSharedFuture<FWwiseLoadedEvent>;
