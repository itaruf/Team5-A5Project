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
#include "Wwise/WwiseFileHandlerBase.h"

#include "WwiseSoundBankManagerImpl.generated.h"

UCLASS()
class WWISEFILEHANDLER_API UWwiseSoundBankManagerImpl : public UEngineSubsystem, public IWwiseSoundBankManager, public FWwiseFileHandlerBase
{
	GENERATED_BODY()

public:
	UWwiseSoundBankManagerImpl();
	~UWwiseSoundBankManagerImpl() override;

	const TCHAR* GetManagingTypeName() const override { return TEXT("SoundBank"); }
	void LoadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FLoadSoundBankCallback&& InCallback) override;
	void UnloadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FUnloadSoundBankCallback&& InCallback) override;
	void SetGranularity(AkUInt32 InStreamingGranularity) override;

	IWwiseStreamingManagerHooks& GetStreamingHooks() final { return *this; }

protected:
	uint32 StreamingGranularity;

	virtual FWwiseFileStateSharedPtr CreateOp(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath);
};
