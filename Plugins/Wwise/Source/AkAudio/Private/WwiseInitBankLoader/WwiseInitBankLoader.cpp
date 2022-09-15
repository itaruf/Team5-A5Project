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
#include "WwiseInitBankLoader/WwiseInitBankLoader.h"

#if WITH_EDITORONLY_DATA
#include "Platforms/AkPlatformInfo.h"
#include "Wwise/WwiseProjectDatabase.h"
#endif

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "FileHelpers.h"
#endif

#include "AkAudioDevice.h"
#include "AkSettings.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "WwiseInitBankLoader"

UWwiseInitBankLoader::UWwiseInitBankLoader()
{
}


void UWwiseInitBankLoader::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

bool UWwiseInitBankLoader::FindInitBank()
{
	FSoftObjectPath Path;
	TArray<FAssetData> InitBankAssets;
	FARFilter Filter;

	Filter.ClassNames.Add(UAkInitBank::StaticClass()->GetFName());

	const auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	AssetRegistryModule.GetAssets(Filter, InitBankAssets);

	if (LIKELY(InitBankAssets.Num() > 0))
	{
		Path = InitBankAssets[0].ToSoftObjectPath();
		UE_CLOG(UNLIKELY(InitBankAssets.Num() > 1), LogAkAudio, Warning, TEXT("Found multiple InitBank assets in project. Randomly selecting one at %s"), *Path.ToString());

		InitBankAsset = Cast<UAkInitBank>(Path.TryLoad());
		UE_CLOG(UNLIKELY(!InitBankAsset), LogAkAudio, Error, TEXT("Could not load InitBank asset at %s"), *Path.ToString());
	}

	if (!InitBankAsset)
	{
		const UAkSettings* Settings = GetDefault<UAkSettings>();
		if (UNLIKELY(!Settings))
		{
			UE_LOG(LogAkAudio, Error, TEXT("Could not retrieve AkSettings"));
			return false;
		}

		Path = Settings->DefaultAssetCreationPath / TEXT("WwiseInitBank");
		InitBankAsset = Cast<UAkInitBank>(Path.TryLoad());
		if (InitBankAsset)
		{
			UE_LOG(LogAkAudio, Verbose, TEXT("WwiseInitBank asset found at %s but not in registry"), *Path.ToString());
		}
		return false;
	}
	return true;
}

#if WITH_EDITORONLY_DATA
void UWwiseInitBankLoader::FindOrCreateInitBank()
{
	if (!FindInitBank())
	{
		const UAkSettings* Settings = GetDefault<UAkSettings>();
		if (UNLIKELY(!Settings))
		{
			UE_LOG(LogAkAudio, Error, TEXT("Could not retrieve AkSettings"));
			return;
		}
		FScopedSlowTask SlowTask(0, LOCTEXT("WwiseInitBankCreating", "Creating WwiseInitBank asset..."));
		UE_LOG(LogAkAudio, Log, TEXT("Creating required WwiseInitBank at %s"), *Settings->DefaultAssetCreationPath);

		auto& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		InitBankAsset = Cast<UAkInitBank>(AssetToolsModule.CreateAsset(TEXT("WwiseInitBank"), Settings->DefaultAssetCreationPath, UAkInitBank::StaticClass(), nullptr));
		UEditorLoadingAndSavingUtils::SavePackages({ InitBankAsset->GetPackage() }, true);
	}
}
#endif


void UWwiseInitBankLoader::LoadInitBank()
{
	if (UNLIKELY(!InitBankAsset))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("LoadInitBank: InitBankAsset not initialized"));
		return;
	}

	UE_LOG(LogAkAudio, Verbose, TEXT("LoadInitBank: Loading init bank asset"));
	InitBankAsset->LoadInitBank(false);
}

void UWwiseInitBankLoader::UnloadInitBank()
{
	if (UNLIKELY(!InitBankAsset))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UnloadInitBank: InitBankAsset not initialized"));
		return;
	}

	UE_LOG(LogAkAudio, Verbose, TEXT("UnloadInitBank: Unloading init bank asset"));
	InitBankAsset->UnloadInitBank();
}

void UWwiseInitBankLoader::ReloadInitBank()
{
	if (UNLIKELY(!InitBankAsset))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("ReloadInitBank: InitBankAsset not initialized"));
		return;
	}

	UE_LOG(LogAkAudio, Verbose, TEXT("ReloadInitBank: Reloading init bank asset"));
	InitBankAsset->LoadInitBank(true);
}
