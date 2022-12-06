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

#include "Wwise/CookedData/WwiseExternalSourceCookedData.h"
#include "CoreMinimal.h"
#include "Async/Future.h"

struct WWISERESOURCELOADER_API FWwiseLoadedExternalSourceInfo
{
	FWwiseLoadedExternalSourceInfo(const FWwiseExternalSourceCookedData& InExternalSource) :
		ExternalSourceCookedData(InExternalSource)
	{}
	FWwiseLoadedExternalSourceInfo& operator=(const FWwiseLoadedExternalSourceInfo&) = delete;

	const FWwiseExternalSourceCookedData ExternalSourceCookedData;

	struct FLoadedData
	{
		FLoadedData() {}
		FLoadedData(const FLoadedData&) = delete;
		FLoadedData& operator=(const FLoadedData&) = delete;

		bool bLoaded = false;

		bool IsLoaded() const
		{
			return bLoaded;
		}
	} LoadedData;

private:
	friend class TDoubleLinkedList<FWwiseLoadedExternalSourceInfo>::TDoubleLinkedListNode;
	FWwiseLoadedExternalSourceInfo(const FWwiseLoadedExternalSourceInfo& InOriginal) :
		ExternalSourceCookedData(InOriginal.ExternalSourceCookedData)
	{
	}
};

using FWwiseLoadedExternalSourceList = TDoubleLinkedList<FWwiseLoadedExternalSourceInfo>;
using FWwiseLoadedExternalSourceListNode = FWwiseLoadedExternalSourceList::TDoubleLinkedListNode;
using FWwiseLoadedExternalSource = FWwiseLoadedExternalSourceListNode*;
using FWwiseLoadedExternalSourcePromise = TPromise<FWwiseLoadedExternalSource>;
using FWwiseLoadedExternalSourceFuture = TSharedFuture<FWwiseLoadedExternalSource>;
