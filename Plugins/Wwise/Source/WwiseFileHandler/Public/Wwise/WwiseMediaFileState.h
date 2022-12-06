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

#include "Wwise/WwiseFileState.h"
#include "Wwise/WwiseStreamableFileStateInfo.h"
#include "Wwise/CookedData/WwiseMediaCookedData.h"

class WWISEFILEHANDLER_API FWwiseMediaFileState : public FWwiseFileState, public FWwiseMediaCookedData
{
public:
	const FString RootPath;

protected:
	FWwiseMediaFileState(const FWwiseMediaCookedData& InCookedData, const FString& InRootPath);

public:
	~FWwiseMediaFileState() override;
	const TCHAR* GetManagingTypeName() const override final { return TEXT("Media"); }
	uint32 GetShortId() const override final { return MediaId; }
};

class WWISEFILEHANDLER_API FWwiseInMemoryMediaFileState : public FWwiseMediaFileState, public AkSourceSettings
{
public:
	FWwiseInMemoryMediaFileState(const FWwiseMediaCookedData& InCookedData, const FString& InRootPath);
	~FWwiseInMemoryMediaFileState() override { Term(); }

	void OpenFile(FOpenFileCallback&& InCallback) override;
	void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) override;
	void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) override;
	void CloseFile(FCloseFileCallback&& InCallback) override;
};

class WWISEFILEHANDLER_API FWwiseStreamingMediaFileState : public FWwiseMediaFileState, protected FWwiseStreamableFileStateInfo
{
public:
	const uint32 StreamingGranularity;

	FWwiseStreamedFile* StreamedFile;

	FWwiseStreamingMediaFileState(const FWwiseMediaCookedData& InCookedData, const FString& InRootPath, uint32 InStreamingGranularity);
	~FWwiseStreamingMediaFileState() override { Term(); }

	uint32 GetPrefetchSize() const;

	void CloseStreaming() override;
	FWwiseStreamableFileStateInfo* GetStreamableFileStateInfo() override { return this; }
	const FWwiseStreamableFileStateInfo* GetStreamableFileStateInfo() const override { return this; }

	bool CanOpenFile() const override;
	void OpenFile(FOpenFileCallback&& InCallback) override;
	void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) override;
	void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) override;
	bool CanCloseFile() const override;
	void CloseFile(FCloseFileCallback&& InCallback) override;

	bool CanProcessFileOp() const override;
	AKRESULT ProcessRead(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, TFileOpDoneCallback&& InFileOpDoneCallback) override;
};
