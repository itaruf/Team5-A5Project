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

#include "Wwise/WwiseFileHandlerModule.h"
#include "Wwise/WwiseStreamableFileHandler.h"

struct FWwiseSoundBankCookedData;

class IWwiseSoundBankManager : public IWwiseStreamableFileHandler
{
public:
	inline static IWwiseSoundBankManager* Get()
	{
		if (auto* Module = IWwiseFileHandlerModule::GetModule())
		{
			return Module->GetSoundBankManager();
		}
		return nullptr;
	}

	using FLoadSoundBankCallback = TUniqueFunction<void(bool bSuccess)>;
	using FUnloadSoundBankCallback = TUniqueFunction<void()>;

	virtual void LoadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FLoadSoundBankCallback&& InCallback) = 0;
	virtual void UnloadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FUnloadSoundBankCallback&& InCallback) = 0;
	virtual void SetGranularity(AkUInt32 Uint32) = 0;

protected:
	~IWwiseSoundBankManager() override {}
};
