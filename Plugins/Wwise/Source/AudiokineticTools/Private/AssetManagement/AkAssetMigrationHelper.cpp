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

#include "AkAssetMigrationHelper.h"

#include "AkAssetDatabase.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkDeprecated.h"
#include "AkAudioBank.h"
#include "AkWaapiClient.h"
#include "AkWaapiUtils.h"
#include "AkMigrationWidgets.h"
#include "AkCustomVersion.h"
#include "AkUnrealEditorHelper.h"
#include "AkUnrealHelper.h"
#include "IAudiokineticTools.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "AssetTools/Public/AssetToolsModule.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "UnrealEd/Public/ObjectTools.h"
#include "UnrealEd/Public/FileHelpers.h"
#include "Misc/FileHelper.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "Interfaces/IMainFrameModule.h"

#define LOCTEXT_NAMESPACE "AkAudio"

namespace AkAssetMigration
{
	void PromptMigration(const FMigrationOptions& MigrationOptions, FMigrationOperations& OutMigrationOperations)
	{
		TSharedPtr<SWindow> Dialog = SNew(SWindow)
			.Title(LOCTEXT("BankMigrationDialog", "Wwise Integration Migration"))
			.SupportsMaximize(false)
			.SupportsMinimize(false)
			.FocusWhenFirstShown(true)
			.HasCloseButton(false)
			.SizingRule(ESizingRule::Autosized);

		TSharedPtr<SMigrationWidget> MigrationWidget;

		Dialog->SetContent(
			SAssignNew(MigrationWidget, SMigrationWidget)
			.Dialog(Dialog)
			.ShowBankTransfer(MigrationOptions.bBanksInProject)
			.ShowDeprecatedAssetCleanup(MigrationOptions.bDeprecatedAssetsInProject)
			.ShowAssetMigration(MigrationOptions.bAssetsNotMigrated)
			.ShowProjectMigration(MigrationOptions.bProjectSettingsNotUpToDate)
			.NumDeprecatedAssets(MigrationOptions.NumDeprecatedAssetsInProject)
		);

		FSlateApplication::Get().AddModalWindow(Dialog.ToSharedRef(), nullptr);

		OutMigrationOperations.BankTransferMethod = MigrationWidget->BankTransferWidget->BankTransferMethod;
		OutMigrationOperations.bDoBankCleanup = MigrationWidget->BankTransferWidget->DeleteSoundBanksCheckBox->IsChecked();
		OutMigrationOperations.bTransferAutoload = MigrationWidget->BankTransferWidget->TransferAutoLoadCheckBox->IsChecked();
		OutMigrationOperations.bDoDeprecatedAssetCleanup = MigrationWidget->DeprecatedAssetCleanupWidget->DeleteAssetsCheckBox->IsChecked();
		OutMigrationOperations.bDoAssetMigration = MigrationWidget->AssetMigrationWidget->MigrateAssetsCheckBox->IsChecked();
		OutMigrationOperations.bDoProjectUpdate = MigrationWidget->ProjectMigrationWidget->AutoMigrateCheckbox->IsChecked();
		OutMigrationOperations.bCancelled = MigrationWidget->bCancel;

		return;
	}

	void FindDeprecatedAssets(TArray<FAssetData>& OutDeprecatedAssets)
	{
		OutDeprecatedAssets.Empty();
		FARFilter Filter;
		Filter.ClassNames.Add(UAkMediaAsset::StaticClass()->GetFName());
		Filter.ClassNames.Add(UAkLocalizedMediaAsset::StaticClass()->GetFName());
		Filter.ClassNames.Add(UAkExternalMediaAsset::StaticClass()->GetFName());
		Filter.ClassNames.Add(UAkFolder::StaticClass()->GetFName());
		Filter.ClassNames.Add(UAkAssetPlatformData::StaticClass()->GetFName());
		
		auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().GetAssets(Filter, OutDeprecatedAssets);
	}
	
	void DeleteDeprecatedAssets(const TArray<FAssetData>& InAssetsToDelete)
	{
		ObjectTools::DeleteAssets(InAssetsToDelete, true);

		const FString MediaFolderpath = FPaths::Combine(AkUnrealEditorHelper::GetLegacySoundBankDirectory(), AkUnrealHelper::MediaFolderName);
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.DeleteDirectoryRecursively(*MediaFolderpath);

		const FString LocalizedFolderPath = FPaths::Combine(AkUnrealEditorHelper::GetLegacySoundBankDirectory(), AkUnrealEditorHelper::LocalizedFolderName);
		PlatformFile.DeleteDirectoryRecursively(*LocalizedFolderPath);
	}

	void FindWwiseAssetsInProject(TArray<FAssetData>& OutWwiseAssets)
	{
		FARFilter Filter;
		Filter.ClassNames.Add(UAkAudioType::StaticClass()->GetFName());
		Filter.bRecursiveClasses =true;
		//We want to delete these asset types during cleanup so no need to dirty them
		Filter.RecursiveClassesExclusionSet.Add(UAkAudioBank::StaticClass()->GetFName());
		Filter.RecursiveClassesExclusionSet.Add(UAkFolder::StaticClass()->GetFName());

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().GetAssets(Filter, OutWwiseAssets);
	}

	bool MigrateWwiseAssets(const TArray<FAssetData> & WwiseAssets, bool bShouldSplitSwitchContainerMedia)
	{
		TArray<UPackage*> WwisePackagesToSave;
		EWwiseEventSwitchContainerLoading SwitchContainerShouldLoad = bShouldSplitSwitchContainerMedia
			? EWwiseEventSwitchContainerLoading::LoadOnReference
			: EWwiseEventSwitchContainerLoading::AlwaysLoad;

		for (auto& Asset : WwiseAssets)
		{
			auto AssetPtr = Asset.GetAsset();

			if (AssetPtr && AssetPtr->GetLinkerCustomVersion(FAkCustomVersion::GUID) < FAkCustomVersion::SSOTAkAssetRefactor)
			{
				if (AssetPtr->GetClass() == UAkAudioEvent::StaticClass())
				{
					UAkAudioEvent* Event = Cast<UAkAudioEvent>(AssetPtr);
					Event->EventInfo.SwitchContainerLoading = SwitchContainerShouldLoad;
				}
				if (AssetPtr->MarkPackageDirty())
				{
					WwisePackagesToSave.Add(AssetPtr->GetPackage());
				}
				else
				{
					UE_LOG(LogAudiokineticTools, Warning, TEXT("Could not dirty asset %s during migration, old data will not be cleared."), *AssetPtr->GetFullName());
				}
			}
		}

		if (WwisePackagesToSave.Num() > 0)
		{
			return UEditorLoadingAndSavingUtils::SavePackages(WwisePackagesToSave, true);
		}
		return true;
	}

	bool MigrateAudioBanks(const EBankTransferMode& TransferMode, const bool& bCleanupAssets, const bool& bWasUsingEBP, const bool& bTransferAutoLoad)
	{
		if (IsRunningCommandlet())
		{
			UE_LOG(LogAudiokineticTools, Log, TEXT("Cannot transfer soundbanks when in commandlet. This can be done at a later time manually."));
			return false;
		}
		TSet<FAssetData> FailedBanks;
		TMap<FString, FBankEntry> BanksToTransfer;
		FillBanksToTransfer(BanksToTransfer);
		if (TransferMode != EBankTransferMode::NoTransfer)
		{
			const bool bIncludeMedia = !bWasUsingEBP;
			TransferUserBanksToWwise(TransferMode, BanksToTransfer, FailedBanks, bIncludeMedia);
		}

		if (bTransferAutoLoad)
		{
			for (auto& Bank : BanksToTransfer)
			{
				UAkAudioBank* BankAsset = Cast<UAkAudioBank>(Bank.Value.BankAssetData.GetAsset());

				TArray<FLinkedAssetEntry>& LinkedAuxBusses = Bank.Value.LinkedAuxBusses;
				TArray<FLinkedAssetEntry>& LinkedEvents = Bank.Value.LinkedEvents;

				for (auto& LinkedAuxBus : LinkedAuxBusses)
				{
					if (!BankAsset->bAutoLoad)
					{
						TArray<FAssetData> OutBusses;
						AkAssetDatabase::Get().FindAssets(LinkedAuxBus.AssetGuid, OutBusses);

						for (FAssetData& OutBus : OutBusses)
						{
							auto* BusAsset = Cast<UAkAuxBus>(OutBus.GetAsset());
							BusAsset->bAutoLoad = false;
							BusAsset->MarkPackageDirty();
						}
					}
				}

				for (auto& LinkedEvent : LinkedEvents)
				{
					if (!BankAsset->bAutoLoad)
					{
						TArray<FAssetData> Events;
						AkAssetDatabase::Get().FindAssets(LinkedEvent.AssetGuid, Events);

						for (FAssetData& Event : Events)
						{
							auto* EventAsset = Cast<UAkAuxBus>(Event.GetAsset());
							EventAsset->bAutoLoad = false;
							EventAsset->MarkPackageDirty();
						}
					}
				}
			}
		}
		
		if (bCleanupAssets)
		{
			TSet<FAssetData> ProjectBanks;
			for (auto Bank : BanksToTransfer)
			{
				ProjectBanks.Add(Bank.Value.BankAssetData);
			}
			TArray<FAssetData> BanksToDelete = ProjectBanks.Difference(FailedBanks).Array();
			ObjectTools::DeleteAssets(BanksToDelete, true);
		}

		if (FailedBanks.Num() > 0)
		{
			return false;
		}
		return true;
	}

	void FillBanksToTransfer(TMap<FString, FBankEntry>& BanksToTransfer)
	{
		auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		auto& AssetRegistry = AssetRegistryModule.Get();

		TArray<FAssetData> Banks;
		AssetRegistry.GetAssetsByClass(UAkAudioBank::StaticClass()->GetFName(), Banks);
		for (FAssetData& BankData : Banks)
		{
			FString BankName = BankData.AssetName.ToString();
			BanksToTransfer.Add(BankName, { BankData, {}, {} });
		}

		TArray< FAssetData> Events;
		AssetRegistry.GetAssetsByClass(UAkAudioEvent::StaticClass()->GetFName(), Events);
		for (FAssetData EventData : Events)
		{
			if (UAkAudioEvent* Event = Cast<UAkAudioEvent>(EventData.GetAsset()))
			{
				if (Event->RequiredBank_DEPRECATED != nullptr)
				{
					FString BankName = Event->RequiredBank_DEPRECATED->GetName();
					FLinkedAssetEntry EventEntry = { Event->GetName(), Event->EventInfo.AssetGuid };

					if (BanksToTransfer.Contains(BankName))
					{
						BanksToTransfer[BankName].LinkedEvents.Add(EventEntry);
					}
					else
					{
						FAssetData BankAssetData = FAssetData(Event->RequiredBank_DEPRECATED);
						BanksToTransfer.Add(BankName, { BankAssetData, {EventEntry}, {} });
					}
				}
			}
		}

		TArray< FAssetData> AuxBusses;
		AssetRegistry.GetAssetsByClass(UAkAuxBus::StaticClass()->GetFName(), AuxBusses);
		for (FAssetData AuxBusData : AuxBusses)
		{
			if (UAkAuxBus* AuxBus = Cast<UAkAuxBus>(AuxBusData.GetAsset()))
			{
				if (AuxBus->RequiredBank_DEPRECATED != nullptr)
				{
					FString BankName = AuxBus->RequiredBank_DEPRECATED->GetName();
					FLinkedAssetEntry AuxBusEntry = { AuxBus->GetName(), AuxBus->AuxBusInfo.AssetGuid };

					if (BanksToTransfer.Contains(BankName))
					{
						BanksToTransfer[BankName].LinkedAuxBusses.Add(AuxBusEntry);
					}
					else
					{
						FAssetData BankAssetData = FAssetData(AuxBus->RequiredBank_DEPRECATED);
						BanksToTransfer.Add(BankName, { BankAssetData, {}, {AuxBusEntry} });
					}
				}
			}
		}
	}

	void TransferUserBanksToWwise(EBankTransferMode TransferMode, const TMap<FString, FBankEntry>& InBanksToTransfer, TSet<FAssetData>& OutFailedBanks, const bool& bIncludeMedia)
	{
		TUniquePtr<IFileHandle> FileWriter;

		if (TransferMode == EBankTransferMode::DefinitionFile)
		{
			FString FileLocation;
			//Popup window to get file location
			if (!GetDefinitionFilePath(FileLocation))
			{
				//User aborted file selection, exit
				return;
			}

			// open file to start writing
			IPlatformFile* PlatformFile = &FPlatformFileManager::Get().GetPlatformFile();;
			FileWriter = TUniquePtr<IFileHandle>(PlatformFile->OpenWrite(*FileLocation));
		}

		for (TPair<FString, FBankEntry > BankEntry : InBanksToTransfer)
		{
			FGuid BankID;
			if (TransferMode == EBankTransferMode::WAAPI)
			{
				if (!CreateBankWaapi(BankEntry.Key, BankEntry.Value, BankID))
				{
					OutFailedBanks.Add(BankEntry.Value.BankAssetData);
					continue;
				}
				if (!SetBankIncludesWaapi(BankEntry.Value, BankID, bIncludeMedia))
				{
					OutFailedBanks.Add(BankEntry.Value.BankAssetData);
				}
			}
			else
			{
				WriteBankDefinition(BankEntry.Value, BankID, FileWriter, bIncludeMedia);
			}
		}

		if (TransferMode == EBankTransferMode::DefinitionFile)
		{
			FileWriter->Flush();
		}
	}

	bool GetDefinitionFilePath(FString& OutFilePath)
	{

		FString FileTypes;
		FString AllExtensions;

		bool bAllowMultiSelect = false;

		FileTypes = TEXT("TSV Files (*.tsv)|*.tsv");

		FString DefaultFolder = "C:";
		FString DefaultFile = "SoundBankDefinitions.tsv";

		// Prompt the user for the filenames
		TArray<FString> OpenFileNames;
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		bool bOpened = false;
		if (DesktopPlatform)
		{
			void* ParentWindowWindowHandle = NULL;

			IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
			const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
			if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
			{
				ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
			}

			const FString Title = TEXT("Choose location for definition file");
			bOpened = DesktopPlatform->SaveFileDialog(
				ParentWindowWindowHandle,
				Title,
				*DefaultFolder,
				*DefaultFile,
				FileTypes,
				bAllowMultiSelect,
				OpenFileNames
			);
		}
		if (bOpened)
		{
			OutFilePath = OpenFileNames[0];
		}

		return bOpened;
	}


	bool WriteBankDefinition(const FBankEntry& BankEntry, FGuid& OutBankGuid, TUniquePtr<IFileHandle>& FileWriter, const bool& bIncludeMedia)
	{
		FString MediaString = bIncludeMedia? "\tMedia": "";
		for (FLinkedAssetEntry Event : BankEntry.LinkedEvents)
		{
			auto Line = BankEntry.BankAssetData.AssetName.ToString() + TEXT("\t\"") + Event.AssetName + TEXT("\"") + TEXT("\tEvent") + MediaString + TEXT("\tStructure") + LINE_TERMINATOR;
			FTCHARToUTF8 Utf8Formatted(*Line);
			FileWriter->Write(reinterpret_cast<const uint8*>(Utf8Formatted.Get()), Utf8Formatted.Length());
		}
		for (FLinkedAssetEntry Bus : BankEntry.LinkedAuxBusses)
		{
			auto Line = BankEntry.BankAssetData.AssetName.ToString() + TEXT("\t-AuxBus\t\"") + Bus.AssetName + TEXT("\"") + MediaString + TEXT("\tStructure") + LINE_TERMINATOR;
			FTCHARToUTF8 Utf8Formatted(*Line);
			FileWriter->Write(reinterpret_cast<const uint8*>(Utf8Formatted.Get()), Utf8Formatted.Length());
		}
		return true;
	}

	bool CreateBankWaapi(const FString& BankName, const FBankEntry& BankEntry, FGuid& OutBankGuid)
	{
		FAkWaapiClient* WaapiClient = FAkWaapiClient::Get();
		if (!WaapiClient)
		{
			return false;
		}

		TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
		Args->SetStringField(WwiseWaapiHelper::PARENT, TEXT("\\SoundBanks\\Default Work Unit"));
		Args->SetStringField(WwiseWaapiHelper::ON_NAME_CONFLICT, WwiseWaapiHelper::RENAME);
		Args->SetStringField(WwiseWaapiHelper::TYPE,  WwiseWaapiHelper::SOUNDBANK_TYPE);
		Args->SetStringField(WwiseWaapiHelper::NAME, BankName);

#if AK_SUPPORT_WAAPI
		TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
		TSharedPtr<FJsonObject> Result;
		if (!WaapiClient->Call(ak::wwise::core::object::create, Args, Options, Result))
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("Failed to create SoundBank for <%s>, it will not be deleted."), *BankEntry.BankAssetData.PackageName.ToString());

			return false;
		}
		FString IdString;
		if (!Result->TryGetStringField(WwiseWaapiHelper::ID, IdString))
		{
			FString ErrorMessage;
			Result->TryGetStringField(TEXT("message"), ErrorMessage);
			UE_LOG(LogAudiokineticTools, Warning, TEXT("Failed to create SoundBank for <%s>, it will not be deleted.\nMessage : <%s>."), *BankEntry.BankAssetData.PackageName.ToString(), *ErrorMessage);

			return false; // error parsing Json
		}

		FGuid::ParseExact(IdString, EGuidFormats::DigitsWithHyphensInBraces, OutBankGuid);
#endif
		return true;
	}

	bool SetBankIncludesWaapi(const FBankEntry& BankEntry, const FGuid& BankId, const bool& bIncludeMedia)
	{
		FAkWaapiClient* WaapiClient = FAkWaapiClient::Get();
		if (!WaapiClient)
		{
			return false;
		}

		TSet<FString> IncludeIds;
		for (FLinkedAssetEntry Event : BankEntry.LinkedEvents)
		{
			if (Event.AssetGuid.IsValid())
			{
				IncludeIds.Add(Event.AssetGuid.ToString(EGuidFormats::DigitsWithHyphensInBraces));
			}
			else
			{
				IncludeIds.Add(TEXT("Event:") + Event.AssetName);
			}
		}

		for (FLinkedAssetEntry AuxBus : BankEntry.LinkedAuxBusses)
		{
			if (AuxBus.AssetGuid.IsValid())
			{
				IncludeIds.Add(AuxBus.AssetGuid.ToString(EGuidFormats::DigitsWithHyphensInBraces));
			}
			else
			{
				IncludeIds.Add(TEXT("AuxBus:") + AuxBus.AssetName);
			}
		}
		if (IncludeIds.Num() < 0)
		{
			return true;
		}

		TArray<TSharedPtr<FJsonValue>> Filters;
		Filters.Add(MakeShared< FJsonValueString>(TEXT("events")));
		Filters.Add(MakeShared< FJsonValueString>(TEXT("structures")));
		if (bIncludeMedia)
		{
			Filters.Add(MakeShared< FJsonValueString>(TEXT("media")));
		}

		TArray<TSharedPtr<FJsonValue>> IncludeIdJson;
		for (const FString IncludedId : IncludeIds)
		{
			TSharedPtr<FJsonObject> IncludedObject = MakeShared< FJsonObject>();
			IncludedObject->SetStringField(WwiseWaapiHelper::OBJECT, IncludedId);
			IncludedObject->SetArrayField(WwiseWaapiHelper::FILTER, Filters);
			IncludeIdJson.Add(MakeShared< FJsonValueObject>(IncludedObject));
		}

		TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
		Args->SetStringField(WwiseWaapiHelper::SOUNDBANK_FIELD, BankId.ToString(EGuidFormats::DigitsWithHyphensInBraces));
		Args->SetStringField(WwiseWaapiHelper::OPERATION, TEXT("add"));
		Args->SetArrayField(WwiseWaapiHelper::INCLUSIONS, IncludeIdJson);

#if AK_SUPPORT_WAAPI
		TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
		TSharedPtr<FJsonObject> Result;
		if (!WaapiClient->Call(ak::wwise::core::soundbank::setInclusions, Args, Options, Result))
		{
			FString ErrorMessage;
			Result->TryGetStringField(TEXT("message"), ErrorMessage);
			UE_LOG(LogAudiokineticTools, Log, TEXT("Failed to include wwise objects in SoundBank <%s>, it will not be deleted.\nMessage : <%s>."), *BankEntry.BankAssetData.PackageName.ToString(), *ErrorMessage);
			return false;
		}
#endif

		return true;
	}
		
	bool MigrateProjectSettings(FString& ProjectContent, const bool& bWasUsingEBP, const bool& bUseGeneratedSubFolders)
	{

		//migrate split media per id
		TArray<PropertyToChange> PropertiesToAdd;
		if (bWasUsingEBP)
		{
			PropertiesToAdd.Add({ TEXT("AutoSoundBankEnabled"), TEXT("True"), TEXT("<Property Name=\"AutoSoundBankEnabled\" Type=\"bool\" Value=\"True\"/>") });
		}

		if (bUseGeneratedSubFolders)
		{
			PropertiesToAdd.Add({ TEXT("MediaAutoBankSubFolders"), TEXT("True"), TEXT("<Property Name=\"MediaAutoBankSubFolders\" Type=\"bool\" Value=\"True\"/>") });
		}

		static const TArray<FString> LogCentralItemsToRemove = 
		{
			TEXT("<IgnoreItem MessageId=\"MediaDuplicated\"/>"),
			TEXT("<IgnoreItem MessageId=\"MediaNotFound\"/>")
		};

		bool bModified = false;
		if (PropertiesToAdd.Num() >0)
		{
			bModified = InsertProperties(PropertiesToAdd, ProjectContent);
		}
		for (const FString& LogItemToRemove : LogCentralItemsToRemove)
		{
			if (ProjectContent.Contains(LogItemToRemove))
			{
				ProjectContent.ReplaceInline(*LogItemToRemove, TEXT(""));
				bModified = true;
			}
		}
		return bModified;
	}
		
	bool SetStandardSettings(FString& ProjectContent)
	{
		static const TArray<PropertyToChange> PropertiesToAdd = {
			{ TEXT("GenerateMultipleBanks"), TEXT("True"), TEXT("<Property Name=\"GenerateMultipleBanks\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("GenerateSoundBankJSON"), TEXT("True"), TEXT("<Property Name=\"GenerateSoundBankJSON\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("SoundBankGenerateEstimatedDuration"), TEXT("True"), TEXT("<Property Name=\"SoundBankGenerateEstimatedDuration\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("SoundBankGenerateMaxAttenuationInfo"), TEXT("True"), TEXT("<Property Name=\"SoundBankGenerateMaxAttenuationInfo\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("SoundBankGeneratePrintGUID"), TEXT("True"), TEXT("<Property Name=\"SoundBankGeneratePrintGUID\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("SoundBankGeneratePrintPath"), TEXT("True"), TEXT("<Property Name=\"SoundBankGeneratePrintPath\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("CopyLooseStreamedMedia"), TEXT("True"), TEXT("<Property Name=\"CopyLooseStreamedMedia\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("RemoveUnusedGeneratedFiles"), TEXT("True"), TEXT("<Property Name=\"RemoveUnusedGeneratedFiles\" Type=\"bool\" Value=\"True\"/>") },
		};

		return InsertProperties(PropertiesToAdd, ProjectContent);
	}

	bool InsertProperties(const TArray<PropertyToChange>& PropertiesToChange, FString& ProjectContent)
	{
		static const auto PropertyListStart = TEXT("<PropertyList>");
		static const FString EndTag = TEXT(">");
		static const TCHAR EmptyElementEndChar = '/';
		static const FString ValueTag = TEXT("<Value>");
		static const FString EndValueTag = TEXT("</Value>");

		static const FString ValueAttribute = TEXT("Value=\"");
		static const FString EndValueAttribute = TEXT("\"");

		bool bModified = false;

		int32 PropertyListPosition = ProjectContent.Find(PropertyListStart);
		if (PropertyListPosition != -1)
		{
			int32 InsertPosition = PropertyListPosition + FCString::Strlen(PropertyListStart);

			for (PropertyToChange ItemToAdd : PropertiesToChange)
			{
				auto idx = ProjectContent.Find(ItemToAdd.Name);
				if (idx == -1)
				{
					ProjectContent.InsertAt(InsertPosition, FString::Printf(TEXT("\n\t\t\t\t%s"), *ItemToAdd.Xml));
					bModified = true;
				}
				else
				{
					FString ValueText;
					FString EndValueText;
					int32 EndTagIdx = ProjectContent.Find(EndTag, ESearchCase::IgnoreCase, ESearchDir::FromStart, idx);
					if (ProjectContent[EndTagIdx - 1] == EmptyElementEndChar)
					{
						// The property is an empty element, the value will be in an attribute
						ValueText = ValueAttribute;
						EndValueText = EndValueAttribute;
					}
					else
					{
						// We are in a ValueList
						ValueText = ValueTag;
						EndValueText = EndValueTag;
					}

					int32 ValueIdx = ProjectContent.Find(ValueText, ESearchCase::IgnoreCase, ESearchDir::FromStart, idx);
					int32 EndValueIdx = ProjectContent.Find(EndValueText, ESearchCase::IgnoreCase, ESearchDir::FromStart, ValueIdx);
					if (ValueIdx != -1 && ValueIdx > idx && ValueIdx < EndValueIdx)
					{
						ValueIdx += ValueText.Len();
						auto ValueEndIdx = ProjectContent.Find(EndValueText, ESearchCase::IgnoreCase, ESearchDir::FromStart, ValueIdx);
						if (ValueEndIdx != -1)
						{
							FString value = ProjectContent.Mid(ValueIdx, ValueEndIdx - ValueIdx);
							if (value != ItemToAdd.Value)
							{
								ProjectContent.RemoveAt(ValueIdx, ValueEndIdx - ValueIdx, false);
								ProjectContent.InsertAt(ValueIdx, ItemToAdd.Value);
								bModified = true;
							}
						}
					}
					else
					{
						UE_LOG(LogAudiokineticTools, Log, TEXT("Could not change value for %s in Wwise project. Some features might not work properly."), *ItemToAdd.Name);
					}
				}
			}
		}

		return bModified;
	}
}

#undef LOCTEXT_NAMESPACE
