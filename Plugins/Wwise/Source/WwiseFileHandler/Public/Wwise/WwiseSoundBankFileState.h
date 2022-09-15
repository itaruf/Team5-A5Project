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

#include "Wwise/WwiseSoundBankManager.h"
#include "Wwise/CookedData/WwiseSoundBankCookedData.h"
#include "Wwise/WwiseFileState.h"

class WWISEFILEHANDLER_API FWwiseSoundBankFileState : public FWwiseFileState, public FWwiseSoundBankCookedData
{
public:
	const FString RootPath;

	FWwiseSoundBankFileState(const FWwiseSoundBankCookedData& InCookedData, const FString& InRootPath);

	const TCHAR* GetManagingTypeName() const override final { return TEXT("SoundBank"); }
	uint32 GetShortId() const override final { return SoundBankId; }
};

class WWISEFILEHANDLER_API FWwiseInMemorySoundBankFileState : public FWwiseSoundBankFileState
{
public:
	const uint8* Ptr;
	int64 FileSize;
	IMappedFileHandle* MappedHandle;
	IMappedFileRegion* MappedRegion;

	FWwiseInMemorySoundBankFileState(const FWwiseSoundBankCookedData& InCookedData, const FString& InRootPath);
	~FWwiseInMemorySoundBankFileState() override { FileStateExecutionQueue.Stop(); }

	bool LoadAsMemoryView() const;

	void OpenFile(FOpenFileCallback&& InCallback) override;
	void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) override;
	void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) override;
	bool CanCloseFile() const override;
	void CloseFile(FCloseFileCallback&& InCallback) override;
};
