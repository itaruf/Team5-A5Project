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

#include "Wwise/WwiseSoundEngineModule.h"
#include "Wwise/LowLevel/WwiseLowLevelComm.h"
#include "Wwise/LowLevel/WwiseLowLevelMemoryMgr.h"
#include "Wwise/LowLevel/WwiseLowLevelMonitor.h"
#include "Wwise/LowLevel/WwiseLowLevelMusicEngine.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"
#include "Wwise/LowLevel/WwiseLowLevelSpatialAudio.h"
#include "Wwise/LowLevel/WwiseLowLevelStreamMgr.h"
#include "Wwise/Stats/SoundEngine.h"
#include "Wwise/WwiseGlobalCallbacks.h"

IMPLEMENT_MODULE(FWwiseSoundEngineModule, WwiseSoundEngine)

FWwiseLowLevelComm* IWwiseSoundEngineModule::Comm = nullptr;
FWwiseLowLevelMemoryMgr* IWwiseSoundEngineModule::MemoryMgr = nullptr;
FWwiseLowLevelMonitor* IWwiseSoundEngineModule::Monitor = nullptr;
FWwiseLowLevelMusicEngine* IWwiseSoundEngineModule::MusicEngine = nullptr;
FWwiseLowLevelSoundEngine* IWwiseSoundEngineModule::SoundEngine = nullptr;
FWwiseLowLevelSpatialAudio* IWwiseSoundEngineModule::SpatialAudio = nullptr;
FWwiseLowLevelStreamMgr* IWwiseSoundEngineModule::StreamMgr = nullptr;
FWwiseGlobalCallbacks* IWwiseSoundEngineModule::GlobalCallbacks = nullptr;

void FWwiseSoundEngineModule::StartupModule()
{
	Comm = new FWwiseLowLevelComm;
	MemoryMgr = new FWwiseLowLevelMemoryMgr;
	Monitor = new FWwiseLowLevelMonitor;
	MusicEngine = new FWwiseLowLevelMusicEngine;
	SoundEngine = new FWwiseLowLevelSoundEngine;
	SpatialAudio = new FWwiseLowLevelSpatialAudio;
	StreamMgr = new FWwiseLowLevelStreamMgr;
	GlobalCallbacks = new FWwiseGlobalCallbacks;
}

void FWwiseSoundEngineModule::ShutdownModule()
{
	if (LIKELY(SoundEngine) && UNLIKELY(SoundEngine->IsInitialized()))
	{
		UE_LOG(LogWwiseSoundEngine, Error, TEXT("Shutting down Sound Engine module without deinitializing Low-Level Sound Engine"));
		SoundEngine->Term();
		GlobalCallbacks->Term();
	}

	delete Comm; Comm = nullptr;
	delete MemoryMgr; MemoryMgr = nullptr;
	delete Monitor; Monitor = nullptr;
	delete MusicEngine; MusicEngine = nullptr;
	delete SoundEngine; SoundEngine = nullptr;
	delete SpatialAudio; SpatialAudio = nullptr;
	delete StreamMgr; StreamMgr = nullptr;

	delete GlobalCallbacks; GlobalCallbacks = nullptr;
}
