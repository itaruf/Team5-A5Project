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

#include "AkSettingsPerUser.h"

#include "AkAudioDevice.h"
#include "Misc/Paths.h"
#include "AkUnrealHelper.h"

#if WITH_EDITOR
#include "AkUnrealEditorHelper.h"
#include "SettingsEditor/Public/ISettingsEditorModule.h"
#endif
//////////////////////////////////////////////////////////////////////////
// UAkSettingsPerUser

UAkSettingsPerUser::UAkSettingsPerUser(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	WwiseWindowsInstallationPath.Path = FPlatformMisc::GetEnvironmentVariable(TEXT("WWISEROOT"));
#endif
}

#if WITH_EDITOR
void UAkSettingsPerUser::PreEditChange(FProperty* PropertyAboutToChange)
{
	PreviousWwiseWindowsInstallationPath = WwiseWindowsInstallationPath.Path;
	PreviousWwiseMacInstallationPath = WwiseMacInstallationPath.FilePath;
	PreviousGeneratedSoundBanksFolder = GeneratedSoundBanksFolderUserOverride.Path;
}

void UAkSettingsPerUser::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const FName MemberPropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, WwiseWindowsInstallationPath))
	{
		AkUnrealEditorHelper::SanitizePath(WwiseWindowsInstallationPath.Path, PreviousWwiseWindowsInstallationPath, FText::FromString("Please enter a valid Wwise Installation path"));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, WwiseMacInstallationPath))
	{
		AkUnrealEditorHelper::SanitizePath(WwiseMacInstallationPath.FilePath, PreviousWwiseMacInstallationPath, FText::FromString("Please enter a valid Wwise Authoring Mac executable path"));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, bAutoConnectToWAAPI))
	{
		OnAutoConnectToWaapiChanged.Broadcast();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, bAutoSyncWwiseAssetNames))
	{
		OnAutoSyncWwiseAssetNamesChanged.Broadcast();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, XmlTranslatorTimeout) || MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, WaapiTranslatorTimeout))
	{
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice)
		{
			AkAudioDevice->SetLocalOutput();
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, GeneratedSoundBanksFolderUserOverride))
	{
		bool bPathChanged = AkUnrealEditorHelper::SanitizeFolderPathAndMakeRelativeToContentDir(
			GeneratedSoundBanksFolderUserOverride.Path, PreviousGeneratedSoundBanksFolder, 
			FText::FromString("Please enter a valid directory path"));

		if (bPathChanged)
		{
			OnGeneratedSoundBanksPathChanged.Broadcast();
		}
		OnGeneratedSoundBanksPathChanged.Broadcast();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
