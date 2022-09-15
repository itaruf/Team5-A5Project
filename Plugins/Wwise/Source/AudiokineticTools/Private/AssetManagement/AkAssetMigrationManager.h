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

#include "AkAssetMigrationHelper.h"
#include "AkSettings.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"

class AkAssetMigrationManager
{
public:
	void Init();
	void Uninit();

	void TryMigration();
	bool IsMigrationRequired(AkAssetMigration::FMigrationOptions& MigrationOptions);
	void CreateMigrationMenuOption();
	void RemoveMigrationMenuOption();

	static void ClearSoundBanksForMigration();
	static void MigrateProjectSettings(const bool& bWasUsingEBP, const bool& bUseGeneratedSubFolders);
	static void SetStandardProjectSettings();
	static bool SetGeneratedSoundBanksPath(const FString& ProjectContent);
	static bool IsSoundDataPathInDirectoriesToAlwaysStage(const FString& SoundDataPath);

private:
	TArray<FAssetData> DeprecatedAssetsInProject;
	TArray<FAssetData> WwiseAssetsInProject;

#if !UE_5_0_OR_LATER
	FLevelEditorModule::FLevelEditorMenuExtender LevelViewportToolbarBuildMenuExtenderAkMigration;
	FDelegateHandle LevelViewportToolbarBuildMenuExtenderAkMigrationHandle;
#endif 
	FName MigrationMenuSectionName = "AkMigration";
};