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
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/CookedData/WwiseExternalSourceCookedData.h"
#include "Wwise/WwiseFileState.h"
#include "Wwise/WwiseFileHandlerBase.h"

#include "WwiseExternalSourceManagerImpl.generated.h"

struct WWISEFILEHANDLER_API FWwiseExternalSourceState : public FWwiseExternalSourceCookedData
{
	FWwiseExternalSourceState(const FWwiseExternalSourceCookedData& InCookedData);
	~FWwiseExternalSourceState();

	int LoadCount;
	void IncrementLoadCount();
	bool DecrementLoadCount();
};
using FWwiseExternalSourceStateSharedPtr = TSharedPtr<FWwiseExternalSourceState>;

UCLASS()
class WWISEFILEHANDLER_API UWwiseExternalSourceManagerImpl : public UEngineSubsystem, public IWwiseExternalSourceManager, public FWwiseFileHandlerBase
{
	GENERATED_BODY()
		 
public:
	UWwiseExternalSourceManagerImpl();
	~UWwiseExternalSourceManagerImpl();

	const TCHAR* GetManagingTypeName() const override { return TEXT("External Source"); }
	void LoadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FLoadExternalSourceCallback&& InCallback) override;
	void UnloadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FUnloadExternalSourceCallback&& InCallback) override;
	void SetGranularity(AkUInt32 InStreamingGranularity) override;

	IWwiseStreamingManagerHooks& GetStreamingHooks() override final { return *this; }

	bool GetExternalSourceInfo(AkExternalSourceInfo& OutInfo, const uint32 InExternalSourceCookie, const FString& InExternalSourceName) override;
	bool GetExternalSourceInfo(AkExternalSourceInfo& OutInfo, const FWwiseExternalSourceCookedData& InCookedData) override;
	bool GetExternalSourceInfos(TArray<AkExternalSourceInfo>& OutInfo, const TArray<FWwiseExternalSourceCookedData>& InCookedData) override;
	bool GetExternalSourceInfos(TArray<AkExternalSourceInfo>& OutInfo, const TArray<uint32> &InExternalSourceCookies, const TArray<FString>& InExternalSourceNames = TArray<FString>()) override;
	void OnPostEvent(const uint32 InPlayingID, const TArray<AkExternalSourceInfo> &InExternalSources) override;
	void OnEndOfEvent(const uint32 InPlayingID) override;

#if WITH_EDITORONLY_DATA
	virtual void Cook(UWwiseResourceCooker& InResourceCooker, const FWwiseExternalSourceCookedData& InCookedData,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile,
		const FWwiseSharedPlatformId& InPlatform, const FWwiseSharedLanguageId& InLanguage) override;
#endif

protected:
	uint32 StreamingGranularity;

	virtual void LoadExternalSourceImpl(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FLoadExternalSourceCallback&& InCallback);
	virtual void UnloadExternalSourceImpl(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FUnloadExternalSourceCallback&& InCallback);
	virtual FWwiseExternalSourceStateSharedPtr CreateExternalSourceState(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath);
	virtual bool CloseExternalSourceState(FWwiseExternalSourceState& InExternalSourceState);

	virtual void LoadExternalSourceMedia(const uint32 InExternalSourceCookie, const FString& InExternalSourceName, const FString& InRootPath, FLoadExternalSourceCallback&& InCallback);
	virtual void UnloadExternalSourceMedia(const uint32 InExternalSourceCookie, const FString& InExternalSourceName, const FString& InRootPath, FUnloadExternalSourceCallback&& InCallback);

	virtual bool GetExternalSourceInfoImpl(AkExternalSourceInfo& OutInfo, uint32 InExternalSourceCookie, const FString& ExternalSourceName);

	TMap<uint32, FWwiseExternalSourceStateSharedPtr> ExternalSourceStatesById;
};
