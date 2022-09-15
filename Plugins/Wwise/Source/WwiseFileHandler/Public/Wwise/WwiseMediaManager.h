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

#include "AkInclude.h"
#include "Wwise/WwiseFileHandlerModule.h"
#include "Wwise/WwiseStreamableFileHandler.h"

struct FWwiseMediaCookedData;

class IWwiseMediaManager : public IWwiseStreamableFileHandler
{
public:
	inline static IWwiseMediaManager* Get()
	{
		if (auto* Module = IWwiseFileHandlerModule::GetModule())
		{
			return Module->GetMediaManager();
		}
		return nullptr;
	}

	using FLoadMediaCallback = TUniqueFunction<void(bool bSuccess)>;
	using FUnloadMediaCallback = TUniqueFunction<void()>;

	virtual void LoadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FLoadMediaCallback&& InCallback) = 0;
	virtual void UnloadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FUnloadMediaCallback&& InCallback) = 0;
	virtual void SetGranularity(AkUInt32 Uint32) = 0;

protected:
	~IWwiseMediaManager() override {}
};
