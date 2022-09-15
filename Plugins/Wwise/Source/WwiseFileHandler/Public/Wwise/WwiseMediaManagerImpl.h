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
#include "Wwise/WwiseMediaManager.h"
#include "Wwise/WwiseFileHandlerBase.h"

#include "WwiseMediaManagerImpl.generated.h"

UCLASS()
class WWISEFILEHANDLER_API UWwiseMediaManagerImpl : public UEngineSubsystem, public IWwiseMediaManager, public FWwiseFileHandlerBase
{
	GENERATED_BODY()

public:
	UWwiseMediaManagerImpl();
	~UWwiseMediaManagerImpl() override;

	const TCHAR* GetManagingTypeName() const override { return TEXT("Media"); }

	void LoadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FLoadMediaCallback&& InCallback) override;
	void UnloadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FUnloadMediaCallback&& InCallback) override;
	void SetGranularity(AkUInt32 InStreamingGranularity) override;

	IWwiseStreamingManagerHooks& GetStreamingHooks() final { return *this; }

protected:
	uint32 StreamingGranularity;

	virtual FWwiseFileStateSharedPtr CreateOp(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath);
};
