/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include "AkWaapiClient.h"
#include "WwiseItemType.h"

#include "CoreMinimal.h"


// Unidirectional sync of asset properties from WAAPI -> Unreal assets
class WaapiRenameWatcher
{
public:
	DECLARE_MULTICAST_DELEGATE_ThreeParams(OnWaapiAssetRenamed, const FGuid&, const FString&, const FString&);

	OnWaapiAssetRenamed OnWaapiAssetRenamedMultiDelegate;

public:
	// Force use of singleton instance

	WaapiRenameWatcher(WaapiRenameWatcher const&) = delete;
	void operator=(WaapiRenameWatcher const&) = delete;

	static WaapiRenameWatcher& Get();
	void Init();
	void UnInit();

private:
	WaapiRenameWatcher() {}

	void SubscribeToWaapiRename();
	void UnsubscribeFromWaapiRename();
	void BindToWaapiClient();
	void UnbindFromWaapiClient();
	void OnWaapiRenamed(uint64_t Id, TSharedPtr<FJsonObject> Response);

private:
	FDelegateHandle ProjectLoadedHandle;
	FDelegateHandle ConnectionLostHandle;
	FDelegateHandle ClientBeginDestroyHandle;

	bool bInitialized = false;

	uint64 IdRenamed = 0;

	const FString UnnamedStateGroupFolder = TEXT("<Unnamed State Group>");

};
