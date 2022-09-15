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

#include "Wwise/CookedData/WwiseLanguageCookedData.h"
#include "CoreMinimal.h"

struct FWwiseLocalizedAuxBusCookedData;

struct WWISERESOURCELOADER_API FWwiseLoadedAuxBus
{
	FWwiseLoadedAuxBus(const FWwiseLocalizedAuxBusCookedData& InAuxBus, const FWwiseLanguageCookedData& InLanguage) :
		LocalizedAuxBusCookedData(InAuxBus),
		LanguageRef(InLanguage),
		bLoaded(true)
	{}
	const FWwiseLocalizedAuxBusCookedData& LocalizedAuxBusCookedData;
	FWwiseLanguageCookedData LanguageRef;
	bool bLoaded;
};
using FWwiseLoadedAuxBusList = TDoubleLinkedList<FWwiseLoadedAuxBus>;
using FWwiseLoadedAuxBusListNode = FWwiseLoadedAuxBusList::TDoubleLinkedListNode;
