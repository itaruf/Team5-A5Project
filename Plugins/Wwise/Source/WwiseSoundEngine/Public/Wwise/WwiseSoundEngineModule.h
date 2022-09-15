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

#include "Modules/ModuleManager.h"

class FWwiseLowLevelComm;
class FWwiseLowLevelMemoryMgr;
class FWwiseLowLevelMonitor;
class FWwiseLowLevelMusicEngine;
class FWwiseLowLevelSoundEngine;
class FWwiseLowLevelSpatialAudio;
class FWwiseLowLevelStreamMgr;

class FWwiseGlobalCallbacks;

class IWwiseSoundEngineModule : public IModuleInterface
{
public:
	static WWISESOUNDENGINE_API FWwiseLowLevelComm* Comm;
	static WWISESOUNDENGINE_API FWwiseLowLevelMemoryMgr* MemoryMgr;
	static WWISESOUNDENGINE_API FWwiseLowLevelMonitor* Monitor;
	static WWISESOUNDENGINE_API FWwiseLowLevelMusicEngine* MusicEngine;
	static WWISESOUNDENGINE_API FWwiseLowLevelSoundEngine* SoundEngine;
	static WWISESOUNDENGINE_API FWwiseLowLevelSpatialAudio* SpatialAudio;
	static WWISESOUNDENGINE_API FWwiseLowLevelStreamMgr* StreamMgr;

	static WWISESOUNDENGINE_API FWwiseGlobalCallbacks* GlobalCallbacks;

	/**
	 * Checks to see if this module is loaded and ready.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(TEXT("WwiseSoundEngine"));
	}

	static void ForceLoadModule()
	{
		FModuleManager& ModuleManager = FModuleManager::Get();
		if (!IsAvailable())
		{
			if (IsEngineExitRequested())
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping reloading missing WwiseSoundEngine: Exiting."));
			}
			else if (!IsInGameThread())
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping loading missing WwiseSoundEngine: Not in game thread"));
			}
			else
			{
				ModuleManager.LoadModule("WwiseSoundEngine");
			}
		}
	}
};

class WWISESOUNDENGINE_API FWwiseSoundEngineModule : public IWwiseSoundEngineModule
{
	void StartupModule() override;
	void ShutdownModule() override;
};
