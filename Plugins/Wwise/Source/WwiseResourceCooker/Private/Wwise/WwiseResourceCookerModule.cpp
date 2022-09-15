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


#include "Wwise/WwiseResourceCookerModule.h"
#include "Interfaces/ITargetPlatform.h"
#include "Wwise/WwiseCookingCache.h"
#include "Wwise/WwiseResourceCookerImpl.h"

#include "Wwise/WwiseResourceLoader.h"

#include "Editor.h"
#include "CookOnTheSide/CookLog.h"
#include "GameDelegates.h"
#include "Wwise/Stats/ResourceCooker.h"

IMPLEMENT_MODULE(FWwiseResourceCookerModuleImpl, WwiseResourceCooker)

FDelegateHandle IWwiseResourceCookerModule::ModifyCookDelegateHandle;

void IWwiseResourceCookerModule::StartupModule()
{
#if UE_5_0_OR_LATER
	// This StartupModule can be executed multiple times as more than one module can derive from the interface.
	// Use GetModule to load what the user wishes, and ignore the current "this".
	auto* This = GetModule();
	if (UNLIKELY(!This))
	{
		return;
	}

	// The act of GetModule() can StartupModule another one. Make sure we are not recursively set.
	if (ModifyCookDelegateHandle.IsValid())
	{
		return;
	}
	ModifyCookDelegateHandle = FGameDelegates::Get().GetModifyCookDelegate().AddRaw(This, &IWwiseResourceCookerModule::OnModifyCook);
#endif
}

void IWwiseResourceCookerModule::ShutdownModule()
{
#if UE_5_0_OR_LATER
	if (!ModifyCookDelegateHandle.IsValid())
	{
		return;
	}

	FGameDelegates::Get().GetModifyCookDelegate().Remove(ModifyCookDelegateHandle);
	ModifyCookDelegateHandle.Reset();
#endif
}


UWwiseResourceCooker* FWwiseResourceCookerModuleImpl::GetDefaultCooker()
{
	if (UNLIKELY(!GEditor))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("Trying to load WwiseResourceCooker subsystem without an editor."));
		return nullptr;
	}
	return GEditor->GetEditorSubsystem<UWwiseResourceCookerImpl>();
}

UWwiseResourceCooker* FWwiseResourceCookerModuleImpl::CreateCookerForPlatform(const ITargetPlatform* TargetPlatform,
	const FWwiseSharedPlatformId& InPlatform, EWwiseExportDebugNameRule InExportDebugNameRule)
{
	{
		const auto* CookingPlatform = CookingPlatforms.Find(TargetPlatform);

		if (UNLIKELY(!CookingPlatform))
		{
#if UE_5_0_OR_LATER
			if (IWwiseProjectDatabaseModule::IsInACookingCommandlet() && !IsRunningCookOnTheFly())		// By The Book cooking needs to predefine the requested platforms. InEditor and OnTheFly should create them all the time.
			{
				UE_LOG(LogWwiseResourceCooker, Warning, TEXT("CreateCookerForPlatform: Not cooking for platform %s"),
					*InPlatform.GetPlatformName());
				return nullptr;
			}
#endif
		}
		else if (*CookingPlatform)
		{
			return *CookingPlatform;
		}
	}

	const auto* DefaultCooker = GetDefaultCooker();
	if (UNLIKELY(!DefaultCooker))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("CreateCookerForPlatform: No Default Cooker available creating platform %s"),
			*InPlatform.GetPlatformName());
		return nullptr;
	}

	const auto* DefaultProjectDatabase = DefaultCooker->GetProjectDatabase();
	if (UNLIKELY(!DefaultProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("CreateCookerForPlatform: No Default ProjectDatabase available creating platform %s"),
			*InPlatform.GetPlatformName());
		return nullptr;
	}

	const auto* DefaultResourceLoader = DefaultProjectDatabase->GetResourceLoader();
	if (UNLIKELY(!DefaultResourceLoader))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("CreateCookerForPlatform: No ResourceLoader available creating platform %s"),
			*InPlatform.GetPlatformName());
		return nullptr;
	}

	const FWwiseResourceLoaderImplScopeLock DefaultResourceLoaderImpl(DefaultResourceLoader);

	UE_LOG(LogWwiseResourceCooker, Display, TEXT("Starting cooking process for Target %s (Wwise platform: %s)"),
		TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("null"),
		*InPlatform.GetPlatformName(), *InPlatform.GetPlatformGuid().ToString());

	auto* NewResourceLoaderImpl = NewObject<UWwiseResourceLoaderImpl>((UObject*)GetTransientPackage(), DefaultResourceLoaderImpl->GetClass());
	NewResourceLoaderImpl->CurrentPlatform = InPlatform;
	NewResourceLoaderImpl->StagePath = DefaultResourceLoaderImpl->StagePath;
#if WITH_EDITORONLY_DATA
	NewResourceLoaderImpl->GeneratedSoundBanksPath = DefaultResourceLoaderImpl->GeneratedSoundBanksPath;
#endif

	auto* NewResourceLoader = NewObject<UWwiseResourceLoader>((UObject*)GetTransientPackage(), DefaultResourceLoader->GetClass());
	NewResourceLoader->LockedResourceLoaderImpl = NewResourceLoaderImpl;
	NewResourceLoaderImpl->CurrentLanguage = NewResourceLoader->SystemLanguage();

	auto* NewProjectDatabase = NewObject<UWwiseProjectDatabase>((UObject*)GetTransientPackage(), DefaultProjectDatabase->GetClass());
	NewProjectDatabase->PrepareProjectDatabaseForPlatform(NewResourceLoader);
	NewProjectDatabase->UpdateDataStructure(nullptr, nullptr);

	auto* NewResourceCooker = NewObject<UWwiseResourceCooker>((UObject*)GetTransientPackage(), DefaultCooker->GetClass());
	NewResourceCooker->PrepareResourceCookerForPlatform(NewProjectDatabase, InExportDebugNameRule);

	CookingPlatforms.Add(TargetPlatform, NewResourceCooker);
	return NewResourceCooker;
}

void FWwiseResourceCookerModuleImpl::DestroyCookerForPlatform(const ITargetPlatform* TargetPlatform)
{
	if (UNLIKELY(!CookingPlatforms.Find(TargetPlatform)))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("DestroyCookerForPlatform: Target %s not created"),
			TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("null"));
		return;
	}

	UE_LOG(LogWwiseResourceCooker, Log, TEXT("DestroyCookerForPlatform for target: %s"),
		TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("null"));

	CookingPlatforms.Remove(TargetPlatform);
}

UWwiseResourceCooker* FWwiseResourceCookerModuleImpl::GetCookerForPlatform(const ITargetPlatform* TargetPlatform)
{
	UWwiseResourceCooker** Result = CookingPlatforms.Find(TargetPlatform);
	if (UNLIKELY(!Result))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetCookerForPlatform: Target %s not created"), *TargetPlatform->PlatformName());
		return nullptr;
	}
	return *Result;
}

void FWwiseResourceCookerModuleImpl::DestroyAllCookerPlatforms()
{
	TArray<const ITargetPlatform*> PreviousTargets;
	CookingPlatforms.GetKeys(PreviousTargets);

	for (const auto* PreviousTarget : PreviousTargets)
	{
		DestroyCookerForPlatform(PreviousTarget);
	}
}

void FWwiseResourceCookerModuleImpl::OnModifyCook(TConstArrayView<const ITargetPlatform*> InTargetPlatforms, TArray<FName>& InOutPackagesToCook, TArray<FName>& InOutPackagesToNeverCook)
{
	DestroyAllCookerPlatforms();

	for (const auto* TargetPlatform : InTargetPlatforms)
	{
		UE_LOG(LogWwiseResourceCooker, Log, TEXT("Starting cooking process for platform %s"), *TargetPlatform->DisplayName().ToString());
		CookingPlatforms.Add(TargetPlatform);
	}
}
