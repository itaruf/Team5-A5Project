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

#include "Wwise/WwiseFileHandlerModule.h"
#include "Wwise/WwiseSoundBankManagerImpl.h"
#include "Wwise/WwiseExternalSourceManagerImpl.h"
#include "Wwise/WwiseMediaManagerImpl.h"
#include "Wwise/WwiseIOHookImpl.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"
#include "Wwise/Stats/FileHandler.h"

IMPLEMENT_MODULE(FWwiseFileHandlerModule, WwiseFileHandler)

FWwiseFileHandlerModule::FWwiseFileHandlerModule()
{
}

IWwiseSoundBankManager* FWwiseFileHandlerModule::GetSoundBankManager()
{
	if (UNLIKELY(!GEngine))
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("Trying to load WwiseSoundBankManager subsystem without an engine."));
		return nullptr;
	}
	return GEngine->GetEngineSubsystem<UWwiseSoundBankManagerImpl>();
}

IWwiseExternalSourceManager* FWwiseFileHandlerModule::GetExternalSourceManager()
{
	if (UNLIKELY(!GEngine))
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("Trying to load WwiseExternalSourceManager subsystem without an engine."));
		return nullptr;
	}
	return GEngine->GetEngineSubsystem<UWwiseExternalSourceManagerImpl>();
}

IWwiseMediaManager* FWwiseFileHandlerModule::GetMediaManager()
{
	if (UNLIKELY(!GEngine))
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("Trying to load WwiseMediaManager subsystem without an engine."));
		return nullptr;
	}
	return GEngine->GetEngineSubsystem<UWwiseMediaManagerImpl>();
}

FWwiseIOHook* FWwiseFileHandlerModule::InstantiateIOHook()
{
	return new FWwiseIOHookImpl;
}
