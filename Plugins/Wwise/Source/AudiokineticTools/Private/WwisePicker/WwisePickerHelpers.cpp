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

#include "WwisePicker/WwisePickerHelpers.h"

#include "AkAcousticTexture.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkRtpc.h"
#include "AkStateValue.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "AkEffectShareSet.h"
#include "AkUnrealHelper.h"
#include "AudiokineticTools/Private/Factories/AkAssetFactories.h"
#include "AudiokineticTools/Private/AssetManagement/AkAssetDatabase.h"
#include "IAudiokineticTools.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetTools/Public/AssetToolsModule.h"
#include "AssetToolsModule.h"
#include "Engine/ObjectReferencer.h"
#include "FileHelpers.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include "UObject/GCObjectScopeGuard.h"

#define LOCTEXT_NAMESPACE "AkAudio"


void WwisePickerHelpers::FindOrCreateAssetsRecursive(const TSharedPtr<FWwiseTreeItem>& WwiseTreeItem,
	TArray<WwisePickerAssetPayload>& InOutPickerAssetPayloads, TSet<FGuid>& InOutKnownGuids,
	const EAssetCreationMode AssetCreationMode, const FString& PackagePath, const FString& CurrentRelativePath)
{
	FString Name;
	UClass* WwiseAssetClass = nullptr;
	FString CurrentRelativePackagePath = PackagePath / CurrentRelativePath;

	if (WwiseTreeItem->ItemId.IsValid() && InOutKnownGuids.Contains(WwiseTreeItem->ItemId))
	{
		return;
	}

	if (WwiseTreeItem->ItemType == EWwiseItemType::Event)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkAudioEvent::StaticClass();
	}
	if (WwiseTreeItem->ItemType == EWwiseItemType::AcousticTexture)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkAcousticTexture::StaticClass();
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::AuxBus)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkAuxBus::StaticClass();
		for (TSharedPtr<FWwiseTreeItem> Child : WwiseTreeItem->GetChildren())
		{
			FString NewRelativePath = CurrentRelativePath / WwiseTreeItem->DisplayName;
			FindOrCreateAssetsRecursive(Child, InOutPickerAssetPayloads, InOutKnownGuids, AssetCreationMode, PackagePath, NewRelativePath);
		}
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::GameParameter)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkRtpc::StaticClass();
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::State)
	{
		Name = FString::Printf(TEXT("%s-%s"), *WwiseTreeItem->Parent.Pin()->DisplayName, *WwiseTreeItem->DisplayName);
		WwiseAssetClass = UAkStateValue::StaticClass();
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::Switch)
	{
		Name = FString::Printf(TEXT("%s-%s"), *WwiseTreeItem->Parent.Pin()->DisplayName, *WwiseTreeItem->DisplayName);
		WwiseAssetClass = UAkSwitchValue::StaticClass();
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::Trigger)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkTrigger::StaticClass();
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::EffectShareSet)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkEffectShareSet::StaticClass();
	}
	else if (IsFolder(WwiseTreeItem))
	{
		//Add  object to prevent Drag and Drop in the world.
		WwiseAssetClass = UAkDragDropBlocker::StaticClass();
		Name = WwiseTreeItem->DisplayName;
		for (TSharedPtr<FWwiseTreeItem> Child : WwiseTreeItem->GetChildren())
		{
			FString NewRelativePath = CurrentRelativePath / Name;
			FindOrCreateAssetsRecursive(Child, InOutPickerAssetPayloads, InOutKnownGuids, AssetCreationMode, PackagePath, NewRelativePath);
		}
	}

	if (WwiseAssetClass)
	{
		TArray<FAssetData> SearchResults;
		WwisePickerAssetPayload Payload;
		AkAssetDatabase::Get().FindAssetsByGuidAndClass(WwiseTreeItem->ItemId, WwiseAssetClass, Payload.ExistingAssets);
		if (Payload.ExistingAssets.Num() == 0)
		{
			Payload.CreatedAsset = CreatePickerAsset(Name, WwiseTreeItem, WwiseAssetClass, AssetCreationMode, PackagePath / CurrentRelativePath);
		}

		Payload.Name = UPackageTools::SanitizePackageName(Name);
		Payload.RelativePackagePath = UPackageTools::SanitizePackageName(CurrentRelativePath);
		Payload.WwiseObjectGuid = WwiseTreeItem->ItemId;
		InOutPickerAssetPayloads.Add(Payload);
		InOutKnownGuids.Add(WwiseTreeItem->ItemId);
	}
}


FAssetData WwisePickerHelpers::CreatePickerAsset(const FString& AssetName, const TSharedPtr<FWwiseTreeItem>& WwiseTreeItem, UClass* AssetClass, const EAssetCreationMode AssetCreationMode, const FString& PackagePath)
{
	//We shouldn't call NewObject outside of the game thread
	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [AssetName, WwiseTreeItem, AssetClass, AssetCreationMode, PackagePath]
		{
			CreatePickerAssetTask(AssetName, WwiseTreeItem, AssetClass, AssetCreationMode, PackagePath);
		});

		// Spoof the FAssetData for the asset that will be created asynchronously
		const FString SanitizedName = UPackageTools::SanitizePackageName(AssetName);
		//Folder asset are always created in the transient package
		if (AssetCreationMode == EAssetCreationMode::Transient || IsFolder(WwiseTreeItem))
		{
			FString AssetPackage = UPackageTools::SanitizePackageName(GetTransientPackage()->GetPathName() / AssetClass->GetName());
#if UE_5_1_OR_LATER
			return FAssetData(FName(AssetPackage), FName(GetTransientPackage()->GetPathName()), FName(*SanitizedName), AssetClass->GetClassPathName());
#else
			return FAssetData(FName(AssetPackage), FName(GetTransientPackage()->GetPathName()), FName(*SanitizedName), AssetClass->GetFName());
#endif
		}
		else if (AssetCreationMode == EAssetCreationMode::InPackage)
		{
			FString AssetPackage = UPackageTools::SanitizePackageName(PackagePath / AssetName);
#if UE_5_1_OR_LATER
			return FAssetData(FName(AssetPackage), FName(PackagePath), FName(*SanitizedName), AssetClass->GetClassPathName());
#else
			return FAssetData(FName(AssetPackage), FName(PackagePath), FName(*SanitizedName), AssetClass->GetFName());
#endif
		}
	}
	return CreatePickerAssetTask(AssetName, WwiseTreeItem, AssetClass, AssetCreationMode, PackagePath);
}


FAssetData WwisePickerHelpers::CreatePickerAssetTask(const FString& AssetName, const TSharedPtr<FWwiseTreeItem>& WwiseTreeItem, UClass* AssetClass, const EAssetCreationMode AssetCreationMode, const FString& PackagePath)
{
	if (!ensureMsgf(IsInGameThread(), TEXT("WwisePickerHelpers::CreatePickerAsset : Not in the Game thread. Assets will not be created.")))
	{
		return {};
	}
	//Folder asset are always created in the transient package
	if (AssetCreationMode == EAssetCreationMode::Transient || IsFolder(WwiseTreeItem))
	{
		return CreateTransientAsset(AssetName, WwiseTreeItem, AssetClass);
	}
	else //if (AssetCreationMode == EAssetCreationMode::InPackage)
	{
		return CreateAssetInPackage(AssetName, WwiseTreeItem, PackagePath, AssetClass);
	}
}

FAssetData WwisePickerHelpers::CreateTransientAsset(const FString& AssetName, const TSharedPtr<FWwiseTreeItem>& WwiseTreeItem, UClass* AssetClass)
{
	//Create a sub-package in transient to avoid asset name collisions for different wwise object type
	const FString PackageName = UPackageTools::SanitizePackageName(GetTransientPackage()->GetPathName() / AssetClass->GetName());
	UPackage* Pkg = CreatePackage(*PackageName);

	UE_LOG(LogAudiokineticTools, VeryVerbose, TEXT("Wwise Picker: Creating new temporary %s asset for Drag operation in '%s' in '%s'."), *AssetClass->GetName(), *AssetName, *PackageName);
	return CreateAsset(AssetName, WwiseTreeItem, AssetClass, Pkg);
}

FAssetData WwisePickerHelpers::CreateAssetInPackage(const FString& AssetName, const TSharedPtr<FWwiseTreeItem>& WwiseTreeItem, const FString& PackagePath, UClass* AssetClass)
{
	const FString PackageName = UPackageTools::SanitizePackageName(PackagePath / AssetName);
	UPackage* Pkg = CreatePackage(*PackageName);

	UE_LOG(LogAudiokineticTools, VeryVerbose, TEXT("Wwise Picker: Creating new %s asset '%s' in '%s'."), *AssetClass->GetName(), *AssetName, *PackageName);
	return CreateAsset(AssetName, WwiseTreeItem, AssetClass, Pkg);
}

FAssetData WwisePickerHelpers::CreateAsset(const FString& AssetName, const TSharedPtr<FWwiseTreeItem>& WwiseTreeItem, UClass* AssetClass, UPackage* Pkg)
{
	// Verify the asset class
	if (!ensureMsgf(AssetClass, TEXT("The new asset '%s' wasn't created due to a problem finding the appropriate class for the new asset.")))
	{
		return nullptr;
	}

	const auto Factory = GetAssetFactory(WwiseTreeItem);
	const FString SanitizedName = UPackageTools::SanitizePackageName(AssetName);
	UObject* NewObj = nullptr;
	EObjectFlags Flags = RF_Public | RF_Transactional | RF_Standalone;
	if (Factory)
	{
		NewObj = Factory->FactoryCreateNew(AssetClass, Pkg, FName(*SanitizedName), Flags, nullptr, GWarn);
	}
	else if (AssetClass)
	{
		NewObj = NewObject<UObject>(Pkg, AssetClass, FName(*SanitizedName), Flags);
	}
	return FAssetData(NewObj);
}

void WwisePickerHelpers::SaveSelectedAssets(TArray<WwisePickerAssetPayload> Assets, const FString& RootPackagePath, const EAssetCreationMode AssetCreationMode, const EAssetDuplicationMode AssetDuplicationMode)
{
	//It is probably dangerous to manipulate UObjects (rename/delete/duplicate) outside of the game thread
	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [Assets, RootPackagePath, AssetCreationMode, AssetDuplicationMode]
		{
			SaveSelectedAssetsTask(Assets, RootPackagePath, AssetCreationMode, AssetDuplicationMode);
		});

		return;
	}
	SaveSelectedAssetsTask(Assets, RootPackagePath, AssetCreationMode, AssetDuplicationMode);
}

void WwisePickerHelpers::SaveSelectedAssetsTask(TArray<WwisePickerAssetPayload> Assets, const FString& RootPackagePath, const EAssetCreationMode AssetCreationMode, const EAssetDuplicationMode AssetDuplicationMode)
{
	if (!ensureMsgf(IsInGameThread(), TEXT("WwisePickerHelpers::SaveSelectedAssets : Not in the Game thread. Assets will not be saved or moved.")))
	{
		return;
	}
	auto AssetToolsModule = &FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<FAssetRenameData> AssetsToRename;
	TArray<FAssetData> AssetsToDelete;
	TArray<FAssetData> RenamedTransientAssets;
	TArray<UPackage*> PackagesToSave;

	for (WwisePickerAssetPayload& AssetResult : Assets)
	{
		const bool bPreExisting = AssetResult.ExistingAssets.Num() > 0;
		FString Path = AssetResult.RelativePackagePath;
		FString PackagePath = RootPackagePath;
		if (!Path.IsEmpty())
		{
			PackagePath = UPackageTools::SanitizePackageName(PackagePath / Path);
		}
		FString NewAssetPath = PackagePath / AssetResult.Name;

		if (bPreExisting)
		{
			if (AssetDuplicationMode == EAssetDuplicationMode::NoDuplication)
			{
				continue;
			}
			//Make sure none of the existing assets would be overwritten by the new asset we want to duplicate
			bool bMatch = false;
			for (auto ExistingAsset : AssetResult.ExistingAssets)
			{
				if (ExistingAsset.PackageName.ToString() == NewAssetPath)
				{
					bMatch = true;
					break;
				}
			}
			if (bMatch)
			{
				//Asset already exists, nothing to do
				continue;
			}
			
			UE_LOG(LogAudiokineticTools, Log, TEXT("Wwise Picker: Duplicating existing asset '%s' into '%s'."), *AssetResult.ExistingAssets[0].GetFullName(), *PackagePath);
			auto NewAsset = AssetToolsModule->Get().DuplicateAsset(AssetResult.Name, PackagePath, AssetResult.ExistingAssets[0].GetAsset());
			if (NewAsset)
			{
				PackagesToSave.Add(NewAsset->GetPackage());
			}
		}
		else
		{
			UObject* NewAsset = AssetResult.CreatedAsset.GetAsset();
			if (IsValid(NewAsset))
			{
				if (NewAsset->IsA<UAkDragDropBlocker>())
				{
					AssetsToDelete.Add(AssetResult.CreatedAsset);
					continue;
				}
			}
			if (AssetCreationMode == EAssetCreationMode::Transient)
			{
				//Drag/Drop assets are created in transient package, and we need to move them to the drop location (or the default folder)
				FAssetRenameData NewAssetRenameData(NewAsset, PackagePath, AssetResult.Name);
				AssetsToRename.Add(NewAssetRenameData);
				RenamedTransientAssets.Add(AssetResult.CreatedAsset);
				UE_LOG(LogAudiokineticTools, Verbose, TEXT("Wwise Picker: Temporary asset '%s' will be moved to '%s'."), *NewAsset->GetFullName(), *PackagePath);
			}
			else if (AssetCreationMode == EAssetCreationMode::InPackage)
			{
				UE_LOG(LogAudiokineticTools, Verbose, TEXT("Wwise Picker: Saving new asset '%s'."), *NewAsset->GetFullName());
				//Assets were created in the destination package but we still need to save them
				PackagesToSave.Add(NewAsset->GetPackage());
			}
		}
	}
	if (AssetsToRename.Num() > 0)
	{
		bool bRenameSuccess = AssetToolsModule->Get().RenameAssets(AssetsToRename);

		//We really don't want to leave assets hanging around in the transient package
		if (!bRenameSuccess && AssetCreationMode == EAssetCreationMode::Transient)
		{
			TArray<UObject*> ObjectsToDelete;
			for (auto Asset : RenamedTransientAssets)
			{
				if (auto TransientObject = Asset.GetAsset())
				{
					ObjectsToDelete.Add(TransientObject);
					UE_LOG(LogAudiokineticTools, Warning, TEXT("Wwise Picker: Failed to rename temporary asset '%s' created in Drag/Drop, it will be deleted."), *Asset.GetFullName());
				}
			}
			ObjectTools::ForceDeleteObjects(ObjectsToDelete);
		}
	}
	if (AssetsToDelete.Num() > 0)
	{
		UE_LOG(LogAudiokineticTools, VeryVerbose, TEXT("Wwise Picker: Deleting '%d' temporary packages."), PackagesToSave.Num());
		ObjectTools::DeleteAssets(AssetsToDelete, false);
	}
	if (PackagesToSave.Num() > 0)
	{
		UE_LOG(LogAudiokineticTools, VeryVerbose, TEXT("Wwise Picker: Saving '%d' new packages."), PackagesToSave.Num());
		UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, true);
	}
}

UAkAssetFactory* WwisePickerHelpers::GetAssetFactory(const TSharedPtr<FWwiseTreeItem>& WwiseTreeItem)
{
	UFactory* Factory = nullptr;
	switch (WwiseTreeItem->ItemType)
	{
	case EWwiseItemType::Event:
		Factory = UAkAudioEventFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::AcousticTexture:
		Factory = UAkAcousticTextureFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::AuxBus:
		Factory = UAkAuxBusFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::GameParameter:
		Factory = UAkRtpcFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::Switch:
		Factory = UAkSwitchValueFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::State:
		Factory = UAkStateValueFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::Trigger:
		Factory = UAkTriggerFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::EffectShareSet:
		Factory = UAkEffectShareSetFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	default:
		return nullptr;
	}
	if (Factory)
	{
		if (auto AkAssetFactory = Cast<UAkAssetFactory>(Factory))
		{
			AkAssetFactory->AssetID = WwiseTreeItem->ItemId;
			AkAssetFactory->WwiseObjectName = WwiseTreeItem->DisplayName;
			return AkAssetFactory;
		}
	}
	return nullptr;
}

bool WwisePickerHelpers::IsFolder(const TSharedPtr<FWwiseTreeItem> Item)
{
	return Item->IsOfType({ EWwiseItemType::Bus, EWwiseItemType::StandaloneWorkUnit, EWwiseItemType::NestedWorkUnit, EWwiseItemType::Folder, EWwiseItemType::PhysicalFolder,  EWwiseItemType::SwitchContainer, EWwiseItemType::SwitchGroup, EWwiseItemType::StateGroup });
}
