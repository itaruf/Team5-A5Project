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

#include "Wwise/CookedData/WwiseGroupValueCookedData.h"
#include "CoreMinimal.h"
#include "Async/Future.h"

struct WWISERESOURCELOADER_API FWwiseLoadedGroupValueInfo
{
	FWwiseLoadedGroupValueInfo(const FWwiseGroupValueCookedData& InGroupValue) :
		GroupValueCookedData(InGroupValue)
	{}
	FWwiseLoadedGroupValueInfo& operator=(const FWwiseLoadedGroupValueInfo&) = delete;

	const FWwiseGroupValueCookedData GroupValueCookedData;

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
	friend class TDoubleLinkedList<FWwiseLoadedGroupValueInfo>::TDoubleLinkedListNode;
	FWwiseLoadedGroupValueInfo(const FWwiseLoadedGroupValueInfo& InOriginal) :
		GroupValueCookedData(InOriginal.GroupValueCookedData)
	{
	}
};

using FWwiseLoadedGroupValueList = TDoubleLinkedList<FWwiseLoadedGroupValueInfo>;
using FWwiseLoadedGroupValueListNode = FWwiseLoadedGroupValueList::TDoubleLinkedListNode;
using FWwiseLoadedGroupValue = FWwiseLoadedGroupValueListNode*;
using FWwiseLoadedGroupValuePromise = TPromise<FWwiseLoadedGroupValue>;
using FWwiseLoadedGroupValueFuture = TSharedFuture<FWwiseLoadedGroupValue>;
