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

#include "HAL/CriticalSection.h"
#include "Misc/Guid.h"
#include "AssetData.h"

class FAssetRegistryModule;
class FAssetToolsModule;
struct FAssetRenameData;

class AUDIOKINETICTOOLS_API AkAssetDatabase
{
public:
	static AkAssetDatabase& Get();

	void Init();
	void BindToWaapiRename();

	void UnbindFromWaapiRename();
	bool FindAllAssets(TArray<FAssetData>& OutData);
	bool FindAssets(const FGuid& AkGuid, TArray<FAssetData>& OutData);
	bool FindAssets(const FString& AkAssetName, TArray<FAssetData>& OutData);
	bool FindFirstAsset(const FGuid& AkGuid, FAssetData& OutAsset);
	bool FindFirstAsset(const FString& AkAssetName, FAssetData& OutAsset);

	bool RenameAsset(const FGuid& Id, const FString& AssetName, const FString& RelativePath);

	void DeleteAsset(const FGuid& Id);
	void DeleteAssets(const TSet<FGuid>& AssetsId);


	void FixUpRedirectors(const FString& AssetPackagePath);

	bool CheckIfLoadingAssets();
	void UnInit();

	mutable FCriticalSection InitBankLock;

private:
	AkAssetDatabase();

	bool IsAkAudioType(const FAssetData& AssetData);
	void OnRenameAsset(const FGuid& Id, const FString& AssetName, const FString& RelativePath);

	FDelegateHandle RenameAssetHandle;

private:
	FAssetRegistryModule* AssetRegistryModule = nullptr;
	FAssetToolsModule* AssetToolsModule = nullptr;
};
