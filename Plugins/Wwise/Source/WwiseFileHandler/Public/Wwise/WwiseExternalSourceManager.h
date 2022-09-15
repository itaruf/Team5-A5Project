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
#include "Wwise/CookedData/WwiseLanguageCookedData.h"

struct FWwiseSharedLanguageId;
struct AkExternalSourceInfo;
struct FWwiseExternalSourceCookedData;
class UWwiseResourceCooker;

#if WITH_EDITORONLY_DATA
struct FWwiseSharedPlatformId;
#endif

class IWwiseExternalSourceManager : public IWwiseStreamableFileHandler
{
public:
	inline static IWwiseExternalSourceManager* Get()
	{
		if (auto* Module = IWwiseFileHandlerModule::GetModule())
		{
			return Module->GetExternalSourceManager();
		}
		return nullptr;
	}

	using FLoadExternalSourceCallback = TUniqueFunction<void(bool bSuccess)>;
	using FUnloadExternalSourceCallback = TUniqueFunction<void()>;

	virtual void LoadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath, 
		const FWwiseLanguageCookedData& InLanguage, FLoadExternalSourceCallback&& InCallback) = 0;
	virtual void UnloadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FUnloadExternalSourceCallback&& InCallback) = 0;
	virtual void SetGranularity(AkUInt32 Uint32) = 0;

	virtual bool GetExternalSourceInfo(AkExternalSourceInfo& OutInfo, const uint32 InExternalSourceCookie, const FString& InExternalSourceName) = 0;
	virtual bool GetExternalSourceInfo(AkExternalSourceInfo& OutInfo, const FWwiseExternalSourceCookedData& InCookedData) = 0;
	virtual bool GetExternalSourceInfos(TArray<AkExternalSourceInfo>& OutInfo, const TArray<FWwiseExternalSourceCookedData>& InCookedData) = 0;
	virtual bool GetExternalSourceInfos(TArray<AkExternalSourceInfo>& OutInfo, const TArray<uint32> &InExternalSourceCookies, const TArray<FString>& InExternalSourceNames = TArray<FString>()) = 0;

	virtual void OnPostEvent(const uint32 InPlayingID, const TArray<AkExternalSourceInfo>& InExternalSources) = 0;
	virtual void OnEndOfEvent(const uint32 InPlayingID) = 0;

#if WITH_EDITORONLY_DATA
	virtual void Cook(UWwiseResourceCooker& InResourceCooker, const FWwiseExternalSourceCookedData& InCookedData, TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile,
		const FWwiseSharedPlatformId& InPlatform, const FWwiseSharedLanguageId& InLanguage) = 0;
#endif

	virtual FString GetStagingDirectory() const { return TEXT("ExternalSources"); }

protected:
	~IWwiseExternalSourceManager() override {}
};
