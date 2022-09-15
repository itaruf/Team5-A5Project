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

#include "AkInclude.h"

namespace AkUnrealHelper
{
	template <typename T>
	struct TMallocDelete
	{
		DECLARE_INLINE_TYPE_LAYOUT(TMallocDelete, NonVirtual);

		TMallocDelete() = default;
		TMallocDelete(const TMallocDelete&) = default;
		TMallocDelete& operator=(const TMallocDelete&) = default;
		~TMallocDelete() = default;

		template <
			typename U,
			typename = decltype(ImplicitConv<T*>((U*)nullptr))
		>
			TMallocDelete(const TMallocDelete<U>&)
		{
		}

		template <
			typename U,
			typename = decltype(ImplicitConv<T*>((U*)nullptr))
		>
			TMallocDelete& operator=(const TMallocDelete<U>&)
		{
			return *this;
		}

		void operator()(T* Ptr) const
		{
			FMemory::Free(Ptr);
		}
	};

	WWISESOUNDENGINE_API void SetHelperFunctions(
		FString(*GetWwisePluginDirectoryImpl)(),
		FString(*GetWwiseProjectPathImpl)(),
		FString(*GetSoundBankDirectoryImpl)(),
		FString(*GetBaseAssetPackagePathImpl)(),
		FString(*GetStagePathImpl)()
		);

	WWISESOUNDENGINE_API FString GetWwisePluginDirectory();
	WWISESOUNDENGINE_API FString GetWwiseProjectPath();
	WWISESOUNDENGINE_API FString GetSoundBankDirectory();
	WWISESOUNDENGINE_API FString GetBaseAssetPackagePath();

	WWISESOUNDENGINE_API void TrimPath(FString& Path);

	WWISESOUNDENGINE_API FString GetProjectDirectory();
	WWISESOUNDENGINE_API FString GetThirdPartyDirectory();
	WWISESOUNDENGINE_API FString GetContentDirectory();
	WWISESOUNDENGINE_API FString GetExternalSourceDirectory();

	WWISESOUNDENGINE_API FString GetLocalizedAssetPackagePath();
	WWISESOUNDENGINE_API FString GetExternalSourceAssetPackagePath();

	WWISESOUNDENGINE_API FString GetWwiseProjectDirectoryPath();
	WWISESOUNDENGINE_API FString GetWwiseSoundBankInfoCachePath();
	WWISESOUNDENGINE_API FString FormatFolderPath(FString folderPath);
	WWISESOUNDENGINE_API bool MakePathRelativeToWwiseProject(FString& AbsolutePath);

	WWISESOUNDENGINE_API const TCHAR* GetResultString(AKRESULT InResult);

	extern WWISESOUNDENGINE_API const TCHAR* MediaFolderName;

	extern WWISESOUNDENGINE_API const FGuid InitBankID;

#if WITH_EDITOR
	WWISESOUNDENGINE_API FString GuidToBankName(const FGuid& Guid);
	WWISESOUNDENGINE_API FGuid BankNameToGuid(const FString& BankName);
#endif
}
