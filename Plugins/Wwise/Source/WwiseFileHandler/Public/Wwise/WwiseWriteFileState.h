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

#include "Wwise/WwiseFileState.h"
#include "Wwise/WwiseStreamableFileStateInfo.h"

class WWISEFILEHANDLER_API FWwiseWriteFileState : public FWwiseFileState, public FWwiseStreamableFileStateInfo
{
public:
	FWwiseWriteFileState(IFileHandle* InFileHandle, const FString& InFilePathName);

	void CloseStreaming() override;

	AKRESULT ProcessWrite(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, TFileOpDoneCallback&& InFileOpDoneCallback) override;

protected:
	IFileHandle* FileHandle;
	FString FilePathName;

	const TCHAR* GetManagingTypeName() const override { return TEXT("Write"); }
	uint32 GetShortId() const override { return 0; }

	void OpenFile(FOpenFileCallback&& InCallback) override { OpenFileSucceeded(MoveTemp(InCallback)); }
	void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) override { LoadInSoundEngineSucceeded(MoveTemp(InCallback)); }
	void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) override { UnloadFromSoundEngineDone(MoveTemp(InCallback)); }
	void CloseFile(FCloseFileCallback&& InCallback) override { CloseFileDone(MoveTemp(InCallback)); }
};