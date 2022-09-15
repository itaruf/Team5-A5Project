#include "WwisePicker/WwisePickerHelpers.h"

#include "AssetToolsModule.h"
#include "AkUnrealHelper.h"
#include "AudiokineticTools/Private/Factories/AkAssetFactories.h"

#include "AkAcousticTexture.h"
#include "AkAssetDropBlocker.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkRtpc.h"
#include "AkStateValue.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "AkEffectShareSet.h"

#include "PackageTools.h"
#include "UObject/GCObjectScopeGuard.h"
#include "AudiokineticTools/Private/AssetManagement/AkAssetDatabase.h"
#include "IAudiokineticTools.h"

#define LOCTEXT_NAMESPACE "AkAudio"

TMap<UObject*, bool> WwisePickerHelpers::RecurseCreateAssets(const TSharedPtr<FWwiseTreeItem>& Asset, const FString& PackagePath, const FString& CurrentPath)
{
	FAssetData SearchResult;
	bool exists = AkAssetDatabase::Get().FindFirstAsset(Asset->ItemId, SearchResult);
	TMap<UObject*, bool> WwiseObjects;
	FString Name;
	UFactory* Factory = nullptr;
	UClass* Class = nullptr;
	if (Asset->ItemType == EWwiseItemType::Event)
	{
		Name = Asset->DisplayName;
		Factory = UAkAudioEventFactory::StaticClass()->GetDefaultObject<UFactory>();
		Class = UAkAudioEvent::StaticClass();
	}
	if (Asset->ItemType == EWwiseItemType::AcousticTexture)
	{
		Name = Asset->DisplayName;
		Factory = UAkAcousticTextureFactory::StaticClass()->GetDefaultObject<UFactory>();
		Class = UAkAcousticTexture::StaticClass();
	}
	else if (Asset->ItemType == EWwiseItemType::AuxBus)
	{
		Name = Asset->DisplayName;
		Factory = UAkAuxBusFactory::StaticClass()->GetDefaultObject<UFactory>();
		Class = UAkAuxBus::StaticClass();
		for (TSharedPtr<FWwiseTreeItem> Child : Asset->GetChildren())
		{
			TMap<UObject*, bool> recursiveObject = RecurseCreateAssets(Child, PackagePath, CurrentPath + "/" + Asset->DisplayName);
			WwiseObjects.Append(recursiveObject);
		}
	}
	else if (Asset->ItemType == EWwiseItemType::GameParameter)
	{
		Name = Asset->DisplayName;
		Factory = UAkRtpcFactory::StaticClass()->GetDefaultObject<UFactory>();
		Class = UAkRtpc::StaticClass();
	}
	else if (Asset->ItemType == EWwiseItemType::State)
	{
		Name = FString::Printf(TEXT("%s-%s"), *Asset->Parent.Pin()->DisplayName, *Asset->DisplayName);
		Factory = UAkStateValueFactory::StaticClass()->GetDefaultObject<UFactory>();
		Class = UAkStateValue::StaticClass();
	}
	else if (Asset->ItemType == EWwiseItemType::Switch)
	{
		Name = FString::Printf(TEXT("%s-%s"), *Asset->Parent.Pin()->DisplayName, *Asset->DisplayName);
		Factory = UAkSwitchValueFactory::StaticClass()->GetDefaultObject<UFactory>();
		Class = UAkSwitchValue::StaticClass();
	}
	else if (Asset->ItemType == EWwiseItemType::Trigger)
	{
		Name = Asset->DisplayName;
		Factory = UAkTriggerFactory::StaticClass()->GetDefaultObject<UFactory>();
		Class = UAkTrigger::StaticClass();
	}
	else if (Asset->ItemType == EWwiseItemType::EffectShareSet)
	{
		Name = Asset->DisplayName;
		Factory = UAkEffectShareSetFactory::StaticClass()->GetDefaultObject<UFactory>();
		Class = UAkEffectShareSet::StaticClass();
	}
	else if (IsFolder(Asset))
	{
		for (TSharedPtr<FWwiseTreeItem> Child : Asset->GetChildren())
		{
			TMap<UObject*, bool> recursiveObject = RecurseCreateAssets(Child, PackagePath, CurrentPath + "/" + Asset->DisplayName);
			WwiseObjects.Append(recursiveObject);
		}
		//Add an AkAudioType object to prevent Drag and Drop in the world.
		UObject* folder = CreateAsset("AkFolder" + Asset->DisplayName, PackagePath, UAkAssetDropBlocker::StaticClass());
		WwiseObjects.Add(folder);
	}

	if (Factory)
	{
		FString Path = PackagePath;
		if (!CurrentPath.IsEmpty())
		{
			Path.RemoveFromEnd("/");
			Path = Path / CurrentPath;
		}

		if (auto akFactory = Cast<UAkAssetFactory>(Factory))
		{
			akFactory->AssetID = Asset->ItemId;
		}

		TCHAR CharString[] = { '\0', '\0' };
		for (const TCHAR* InvalidCharacters = INVALID_LONGPACKAGE_CHARACTERS; *InvalidCharacters; ++InvalidCharacters)
		{
			CharString[0] = *InvalidCharacters;
			Path.ReplaceInline(CharString, TEXT(""));
		}

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked< FAssetToolsModule >("AssetTools");
		WwiseObjects.Add(AssetToolsModule.Get().CreateAsset(Name, Path, Class, Factory), exists);

		return WwiseObjects;
	}

	return WwiseObjects;
}

UObject* WwisePickerHelpers::CreateAsset(const FString& AssetName, const FString& PackagePath, UClass* AssetClass)
{
	// Verify the asset class
	if (!ensure(AssetClass))
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("The new asset wasn't created due to a problem finding the appropriate class for the new asset."));
		return nullptr;
	}

	const FString PackageName = UPackageTools::SanitizePackageName(PackagePath + TEXT("/") + AssetName);

	UClass* ClassToUse = AssetClass;

	UPackage* Pkg = CreatePackage(*PackageName);
	UObject* NewObj = nullptr;
	EObjectFlags Flags = RF_Public | RF_Transactional | RF_Transient;
	if (AssetClass)
	{
		NewObj = NewObject<UObject>(Pkg, ClassToUse, FName(*AssetName), Flags);
	}
	return NewObj;
}

bool WwisePickerHelpers::IsFolder(const TSharedPtr<FWwiseTreeItem> Item)
{
	return Item->IsOfType({ EWwiseItemType::Bus, EWwiseItemType::StandaloneWorkUnit, EWwiseItemType::NestedWorkUnit, EWwiseItemType::Folder, EWwiseItemType::PhysicalFolder,  EWwiseItemType::SwitchContainer, EWwiseItemType::SwitchGroup, EWwiseItemType::StateGroup });
}
