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
#include "Wwise/WwiseExternalSourceManager.h"
#include "Engine/DataTable.h"
#include "Engine/StreamableManager.h"
#include "UObject/StrongObjectPtr.h"
#include "Wwise/WwiseExternalSourceManagerImpl.h"
#include "WwiseSimpleExtSrcManager.generated.h"

struct FWwiseExternalSourceMediaInfo;

UCLASS()
class WWISESIMPLEEXTERNALSOURCE_API UWwiseSimpleExtSrcManager :  public UWwiseExternalSourceManagerImpl
{
	GENERATED_BODY()

public:
	UWwiseSimpleExtSrcManager(){};
	~UWwiseSimpleExtSrcManager(){};

	void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void LoadMediaTables();
	virtual void ReloadExternalSources();

	void OnPostEvent(const uint32 InPlayingID, const TArray<AkExternalSourceInfo>& InExternalSources) override;
	void OnEndOfEvent(const uint32 InPlayingID) override;

	FString GetStagingDirectory() const override;

	UFUNCTION(BlueprintCallable, Category="WwiseExternalSources")
	virtual void SetExternalSourceMediaById(const FString& ExternalSourceName, const int32 MediaId);

	UFUNCTION(BlueprintCallable, Category="WwiseExternalSources")
	virtual void SetExternalSourceMediaByName(const FString& ExternalSourceName, const FString& MediaName);

	UFUNCTION(BlueprintCallable, Category="WwiseExternalSources")
	virtual void SetExternalSourceMediaWithIds(const int32 ExternalSourceCookie, const int32 MediaId);

	#if WITH_EDITORONLY_DATA
	void Cook(UWwiseResourceCooker& InResourceCooker, const FWwiseExternalSourceCookedData& InCookedData, 
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile,
		const FWwiseSharedPlatformId& InPlatform, const FWwiseSharedLanguageId& InLanguage) override;
	#endif

protected:
	void LoadExternalSourceMedia(const uint32 InExternalSourceCookie, const FString& InExternalSourceName, const FString& InRootPath, FLoadExternalSourceCallback&& InCallback) override;
	void UnloadExternalSourceMedia(const uint32 InExternalSourceCookie, const FString& InExternalSourceName, const FString& InRootPath, FUnloadExternalSourceCallback&& InCallback) override;
	bool GetExternalSourceInfoImpl(AkExternalSourceInfo& OutInfo, uint32 InExternalSourceCookie, const FString& InExternalSourceName) override;

	virtual void OnTablesChanged();
	virtual void OnMediaInfoTableChanged();
	virtual void OnDefaultExternalSourceTableChanged();
	virtual void FillExternalSourceToMediaMap(const UDataTable& InMappingTable);
	virtual void FillMediaNameToIdMap(const UDataTable& InMappingTable);

	virtual void SetExternalSourceMedia(const uint32 ExternalSourceCookie, const uint32 MediaInfoId, const FString& ExternalSourceName = TEXT("Unknown"));
	virtual FWwiseFileStateSharedPtr CreateOp(const FWwiseExternalSourceMediaInfo& ExternalSourceMediaInfo, const FString& InRootPath);

protected:
	TStrongObjectPtr<UDataTable> MediaInfoTable;
	TStrongObjectPtr<UDataTable> ExternalSourceDefaultMedia;
	FStreamableManager StreamableManager;
	TMap<uint32, uint32> CookieToMediaId;
	TMap<FString, uint32> MediaNameToId;
	TMultiMap<uint32, uint32> PlayingIdToMediaIds;

	//We cook all media in one shot, so we use this to track whether this cooking has been performed yet
	bool bCooked = false;
};

