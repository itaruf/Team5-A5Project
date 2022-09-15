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

#if WITH_EDITOR

#include "Engine/EngineTypes.h"
#include "Input/Reply.h"

DECLARE_DELEGATE_RetVal(FReply, FOnButtonClickedMigration);

namespace AkUnrealEditorHelper
{
	AKAUDIO_API void ShowEventBasedPackagingMigrationDialog(FOnButtonClickedMigration in_OnclickedYes, FOnButtonClickedMigration in_OnclickedNo);
	AKAUDIO_API void SanitizePath(FString& Path, const FString& PreviousPath, const FText& DialogMessage);
	AKAUDIO_API bool SanitizeFolderPathAndMakeRelativeToContentDir(FString& Path, const FString& PreviousPath, const FText& DialogMessage);

	AKAUDIO_API void SaveConfigFile(UObject* ConfigObject);
	AKAUDIO_API void DeleteOldSoundBanks();
	
	AKAUDIO_API FString GetLegacySoundBankDirectory();
	AKAUDIO_API FString GetContentDirectory();
	AKAUDIO_API void DeleteLegacySoundBanks();

	extern AKAUDIO_API const TCHAR* LocalizedFolderName;
}
#endif
