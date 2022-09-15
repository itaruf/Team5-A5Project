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

#include "Wwise/WwiseProjectDatabaseImpl.h"

#include "Wwise/Metadata/WwiseMetadataPlatformInfo.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseProjectDatabaseDelegates.h"

#include "Async/Async.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "WwiseProjectDatabase"

UWwiseProjectDatabaseImpl::UWwiseProjectDatabaseImpl() :
	ResourceLoaderOverride(nullptr),
	LockedDataStructure(new FWwiseDataStructure())
{
}

UWwiseProjectDatabaseImpl::~UWwiseProjectDatabaseImpl()
{
}

void UWwiseProjectDatabaseImpl::UpdateDataStructure(const FDirectoryPath* InUpdateGeneratedSoundBanksPath, const FGuid* InBasePlatformGuid)
{
	FWwiseSharedPlatformId Platform;
	FDirectoryPath SourcePath;
	{
		auto* ResourceLoader = GetResourceLoader();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}
		if (InUpdateGeneratedSoundBanksPath)
		{
			FWwiseResourceLoaderImplWriteScopeLock ResourceLoaderImpl(ResourceLoader);

			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("UpdateDataStructure: Setting Generated SoundBanksPath to %s"),
				*InUpdateGeneratedSoundBanksPath->Path);
			ResourceLoaderImpl->GeneratedSoundBanksPath = *InUpdateGeneratedSoundBanksPath;
		}

		const FWwiseResourceLoaderImplScopeLock ResourceLoaderImpl(ResourceLoader);
		Platform = ResourceLoaderImpl->CurrentPlatform;
		SourcePath = ResourceLoaderImpl->GeneratedSoundBanksPath;
	}

	{
		FWriteScopeLock WLock(LockedDataStructure->Lock);
		auto& DataStructure = LockedDataStructure.Get();

		if (DisableDefaultPlatforms())
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("UpdateDataStructure: Retrieving root data structure in (%s)"), *SourcePath.Path);
			FScopedSlowTask SlowTask(0, LOCTEXT("WwiseProjectDatabaseUpdate", "Retrieving Wwise data structure root..."));

			{
				FWwiseDataStructure UpdatedDataStructure(SourcePath, nullptr, nullptr);
				DataStructure = MoveTemp(UpdatedDataStructure);
			}
		}
		else
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("UpdateDataStructure: Retrieving data structure for %s (Base: %s) in (%s)"),
				*Platform.GetPlatformName(), InBasePlatformGuid ? *InBasePlatformGuid->ToString() : TEXT("null"), *SourcePath.Path);
			FScopedSlowTask SlowTask(0, FText::Format(
				LOCTEXT("WwiseProjectDatabaseUpdate", "Retrieving Wwise data structure for platform {0}..."),
				FText::FromString(Platform.GetPlatformName())));

			{
				FWwiseDataStructure UpdatedDataStructure(SourcePath, &Platform.GetPlatformName(), InBasePlatformGuid);
				DataStructure = MoveTemp(UpdatedDataStructure);

				// Update platform according to data found if different
				FWwiseSharedPlatformId FoundSimilarPlatform = Platform;
				for (const auto& LoadedPlatform : DataStructure.Platforms)
				{
					FoundSimilarPlatform = LoadedPlatform.Key;
					if (FoundSimilarPlatform == Platform)
					{
						break;
					}
				}

				//Update SharedPlatformId with parsed root paths
				if (DataStructure.Platforms.Contains(FoundSimilarPlatform) )
				{
					const FWwisePlatformDataStructure& PlatformEntry = DataStructure.Platforms.FindRef(FoundSimilarPlatform);
					FoundSimilarPlatform.Platform->ExternalSourceRootPath = PlatformEntry.PlatformRef.GetPlatformInfo()->RootPaths.ExternalSourcesOutputRoot;
				}
				//Update the resource loader current platform as internal data may have changed
				auto* ResourceLoader = GetResourceLoader();
				if (UNLIKELY(!ResourceLoader))
				{
					return;
				}

				ResourceLoader->SetPlatform(FoundSimilarPlatform);
			}

			if (UNLIKELY(DataStructure.Platforms.Num() == 0))
			{
				UE_LOG(LogWwiseProjectDatabase, Error, TEXT("UpdateDataStructure: Could not find suitable platform for %s (Base: %s) in (%s)"),
					*Platform.GetPlatformName(), InBasePlatformGuid ? *InBasePlatformGuid->ToString() : TEXT("null"), *SourcePath.Path);
				return;
			}
		}

		UE_LOG(LogWwiseProjectDatabase, Log, TEXT("UpdateDataStructure: Done."));
	}
	if (Get() == this)		// Only broadcast database updates on main project.
	{
		FWwiseProjectDatabaseDelegates::Get().GetOnDatabaseUpdateCompletedDelegate().Broadcast();
	}
}

void UWwiseProjectDatabaseImpl::PrepareProjectDatabaseForPlatform(UWwiseResourceLoader* InResourceLoader)
{
	ResourceLoaderOverride = InResourceLoader;
}

UWwiseResourceLoader* UWwiseProjectDatabaseImpl::GetResourceLoader()
{
	if (ResourceLoaderOverride)
	{
		return ResourceLoaderOverride;
	}
	else
	{
		return UWwiseResourceLoader::Get();
	}
}

const UWwiseResourceLoader* UWwiseProjectDatabaseImpl::GetResourceLoader() const
{
	if (ResourceLoaderOverride)
	{
		return ResourceLoaderOverride;
	}
	else
	{
		return UWwiseResourceLoader::Get();
	}
}
