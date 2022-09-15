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

#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Misc/Guid.h"
#include "AssetData.h"

struct PropertyToChange;

namespace AkAssetMigration

{
	enum EBankTransferMode
	{
		NoTransfer,
		WAAPI,
		DefinitionFile,
	};

	struct FMigrationOperations
	{
		bool bDoDeprecatedAssetCleanup =false;
		bool bDoAssetMigration=false;
		bool bDoBankCleanup=false;
		bool bTransferAutoload;
		bool bDoProjectUpdate=false;
		bool bCancelled = false;
		EBankTransferMode BankTransferMethod  = EBankTransferMode::NoTransfer;
	};

	struct FMigrationOptions
	{
		bool bBanksInProject;
		bool bDeprecatedAssetsInProject;
		bool bAssetsNotMigrated;
		bool bProjectSettingsNotUpToDate;
		int NumDeprecatedAssetsInProject;
	};

	void PromptMigration(const FMigrationOptions& MigrationOptions, FMigrationOperations& OutMigrationOperations);
	bool MigrateAudioBanks(const EBankTransferMode& TransferMode, const bool& bCleanupAssets, const bool& bWasUsingEBP, const bool& bTransferAutoLoad);
	bool MigrateWwiseAssets(const TArray<FAssetData> & WwiseAssets, bool bShouldSplitSwitchContainerMedia);
	void FindWwiseAssetsInProject(TArray<FAssetData>& OutFoundAssets);

	void FindDeprecatedAssets(TArray<FAssetData>& OutDeprecatedAssets);
	void DeleteDeprecatedAssets(const TArray<FAssetData>& InAssetsToDelete);

	struct FLinkedAssetEntry
	{
		FString AssetName;
		FGuid AssetGuid;
	};

	struct FBankEntry
	{
		FAssetData BankAssetData;
		TArray<FLinkedAssetEntry> LinkedEvents;
		TArray<FLinkedAssetEntry> LinkedAuxBusses;
	};


	void FillBanksToTransfer(TMap<FString, FBankEntry>& BanksToTransfer);
	void TransferUserBanksToWwise(EBankTransferMode TransferMode, const TMap<FString, FBankEntry>& BanksToTransfer, TSet<FAssetData>& FailedBanks, const bool& bIncludeMedia);

	bool GetDefinitionFilePath(FString& OutFilePath);
	bool WriteBankDefinition(const FBankEntry& BankEntry, FGuid& OutBankGuid, TUniquePtr<IFileHandle>& FileWriter,  const bool& bIncludeMedia);
	bool CreateBankWaapi(const FString& BankName, const FBankEntry& BankEntry, FGuid& OutBankGuid);
	bool SetBankIncludesWaapi(const FBankEntry& BankEntry, const FGuid& BankId, const bool& bIncludeMedia);


	bool MigrateProjectSettings(FString& ProjectContent, const bool& bWasUsingEBP,  const bool& bUseGeneratedSubFolders);
	bool SetStandardSettings(FString& ProjectContent);

	
	struct PropertyToChange
	{
		FString Name;
		FString Value;
		FString Xml;

		PropertyToChange(FString n, FString v, FString x)
			: Name(n)
			, Value(v)
			, Xml(x)
		{}
	};

	bool InsertProperties(const TArray<PropertyToChange>& PropertiesToChange, FString& ProjectContent);



};