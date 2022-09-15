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


#include "InitializationSettings/AkInitializationSettings.h"

#include "Platforms/AkUEPlatform.h"
#include "HAL/PlatformMemory.h"

#include "AkAudioDevice.h"
#include "Wwise/WwiseIOHook.h"
#include "Wwise/LowLevel/WwiseLowLevelComm.h"
#include "Wwise/LowLevel/WwiseLowLevelMemoryMgr.h"
#include "Wwise/LowLevel/WwiseLowLevelMonitor.h"
#include "Wwise/LowLevel/WwiseLowLevelMusicEngine.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"
#include "Wwise/LowLevel/WwiseLowLevelSpatialAudio.h"
#include "Wwise/LowLevel/WwiseLowLevelStreamMgr.h"
#include "Wwise/WwiseGlobalCallbacks.h"

namespace AkInitializationSettings_Helpers
{
	enum { IsLoggingInitialization = true };

	void AssertHook(const char* expression, const char* fileName, int lineNumber)
	{
		check(expression);
		check(fileName);
		const FString Expression(expression);
		const FString FileName(fileName);
		UE_LOG(LogAkAudio, Error, TEXT("AKASSERT: %s. File: %s, line: %d"), *Expression, *FileName, lineNumber);
	}

	FAkInitializationStructure::MemoryAllocFunction AllocFunction = nullptr;
	FAkInitializationStructure::MemoryFreeFunction FreeFunction = nullptr;

	void* AkMemAllocVM(size_t size, size_t* /*extra*/)
	{
		return AllocFunction(size);
	}

	void AkMemFreeVM(void* address, size_t /*size*/, size_t /*extra*/, size_t release)
	{
		if (release)
		{
			FreeFunction(address, release);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// FAkInitializationStructure

FAkInitializationStructure::FAkInitializationStructure()
{
	auto* Comm = FWwiseLowLevelComm::Get();
	auto* MemoryMgr = FWwiseLowLevelMemoryMgr::Get();
	auto* MusicEngine = FWwiseLowLevelMusicEngine::Get();
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	auto* StreamMgr = FWwiseLowLevelStreamMgr::Get();
	if (UNLIKELY(!Comm || !MemoryMgr || !MusicEngine || !SoundEngine || !StreamMgr)) return;

	MemoryMgr->GetDefaultSettings(MemSettings);

	StreamMgr->GetDefaultSettings(StreamManagerSettings);

	StreamMgr->GetDefaultDeviceSettings(DeviceSettings);
	DeviceSettings.uSchedulerTypeFlags = AK_SCHEDULER_DEFERRED_LINED_UP;
	DeviceSettings.uMaxConcurrentIO = AK_UNREAL_MAX_CONCURRENT_IO;

	SoundEngine->GetDefaultInitSettings(InitSettings);
	InitSettings.pfnAssertHook = AkInitializationSettings_Helpers::AssertHook;
	InitSettings.eFloorPlane = AkFloorPlane_XY;
	InitSettings.fGameUnitsToMeters = 100.f;

	SoundEngine->GetDefaultPlatformInitSettings(PlatformInitSettings);

	MusicEngine->GetDefaultInitSettings(MusicSettings);

#if AK_ENABLE_COMMUNICATION
	Comm->GetDefaultInitSettings(CommSettings);
#endif
}

FAkInitializationStructure::~FAkInitializationStructure()
{
	delete[] InitSettings.szPluginDLLPath;
}

void FAkInitializationStructure::SetPluginDllPath(const FString& PlatformArchitecture)
{
	auto& PluginDllPath = InitSettings.szPluginDLLPath;
	delete[] PluginDllPath;
	if (PlatformArchitecture.IsEmpty())
	{
		PluginDllPath = nullptr;
		return;
	}

	auto Path = FAkPlatform::GetDSPPluginsDirectory(PlatformArchitecture);
	auto Length = Path.Len() + 1;
	PluginDllPath = new AkOSChar[Length];
	AKPLATFORM::SafeStrCpy(PluginDllPath, TCHAR_TO_AK(*Path), Length);
}

void FAkInitializationStructure::SetupLLMAllocFunctions(MemoryAllocFunction alloc, MemoryFreeFunction free, bool UseMemTracker)
{
	ensure(alloc == nullptr && free == nullptr || alloc != nullptr && free != nullptr);

	AkInitializationSettings_Helpers::AllocFunction = alloc;
	AkInitializationSettings_Helpers::FreeFunction = free;

#if ENABLE_LOW_LEVEL_MEM_TRACKER
	if (UseMemTracker)
	{
		int32 OutAlignment = 0;
		FPlatformMemory::GetLLMAllocFunctions(AkInitializationSettings_Helpers::AllocFunction, AkInitializationSettings_Helpers::FreeFunction, OutAlignment);
	}
#endif

	if (!AkInitializationSettings_Helpers::AllocFunction || !AkInitializationSettings_Helpers::FreeFunction)
		return;

	MemSettings.pfAllocVM = AkInitializationSettings_Helpers::AkMemAllocVM;
	MemSettings.pfFreeVM = AkInitializationSettings_Helpers::AkMemFreeVM;
}


//////////////////////////////////////////////////////////////////////////
// FAkMainOutputSettings

void FAkMainOutputSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return;

	auto& OutputSettings = InitializationStructure.InitSettings.settingsMainOutput;

	auto sharesetID = !AudioDeviceShareset.IsEmpty() ? SoundEngine->GetIDFromString(TCHAR_TO_ANSI(*AudioDeviceShareset)) : AK_INVALID_UNIQUE_ID;
	OutputSettings.audioDeviceShareset = sharesetID;

	switch (ChannelConfigType)
	{
	case EAkChannelConfigType::Anonymous:
		OutputSettings.channelConfig.SetAnonymous(NumberOfChannels);
		break;

	case EAkChannelConfigType::Standard:
		OutputSettings.channelConfig.SetStandard(ChannelMask);
		break;

	case EAkChannelConfigType::Ambisonic:
		OutputSettings.channelConfig.SetAmbisonic(NumberOfChannels);
		break;
	}

	OutputSettings.ePanningRule = (AkPanningRule)PanningRule;
	OutputSettings.idDevice = DeviceID;
}


//////////////////////////////////////////////////////////////////////////
// FAkSpatialAudioSettings

void FAkSpatialAudioSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto& SpatialAudioInitSettings = InitializationStructure.SpatialAudioInitSettings;
	SpatialAudioInitSettings.uMaxSoundPropagationDepth = MaxSoundPropagationDepth;
	SpatialAudioInitSettings.fMovementThreshold = MovementThreshold;
	SpatialAudioInitSettings.uNumberOfPrimaryRays = NumberOfPrimaryRays;
	SpatialAudioInitSettings.uMaxReflectionOrder = ReflectionOrder;
	SpatialAudioInitSettings.uMaxDiffractionOrder = DiffractionOrder;
	SpatialAudioInitSettings.uDiffractionOnReflectionsOrder = DiffractionOnReflectionsOrder;
	SpatialAudioInitSettings.fMaxPathLength = MaximumPathLength;
	SpatialAudioInitSettings.fCPULimitPercentage = CPULimitPercentage;
	SpatialAudioInitSettings.uLoadBalancingSpread = LoadBalancingSpread;
	SpatialAudioInitSettings.bEnableGeometricDiffractionAndTransmission = EnableGeometricDiffractionAndTransmission;
	SpatialAudioInitSettings.bCalcEmitterVirtualPosition = CalcEmitterVirtualPosition;
}


//////////////////////////////////////////////////////////////////////////
// FAkCommunicationSettings

void FAkCommunicationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
#ifndef AK_OPTIMIZED
	auto& CommSettings = InitializationStructure.CommSettings;
	CommSettings.ports.uDiscoveryBroadcast = DiscoveryBroadcastPort;
	CommSettings.ports.uCommand = CommandPort;

	const FString GameName = GetCommsNetworkName();
	FCStringAnsi::Strcpy(CommSettings.szAppNetworkName, AK_COMM_SETTINGS_MAX_STRING_SIZE, TCHAR_TO_ANSI(*GameName));
#endif // AK_OPTIMIZED
}

FString FAkCommunicationSettings::GetCommsNetworkName() const
{
	FString CommsNetworkName = NetworkName;

	if (CommsNetworkName.IsEmpty() && FApp::HasProjectName())
	{
		CommsNetworkName = FApp::GetProjectName();
	}

#if WITH_EDITORONLY_DATA
	if (!CommsNetworkName.IsEmpty() && !IsRunningGame())
	{
		CommsNetworkName += TEXT(" (Editor)");
	}
#endif

	return CommsNetworkName;
}


//////////////////////////////////////////////////////////////////////////
// FAkCommunicationSettingsWithSystemInitialization

void FAkCommunicationSettingsWithSystemInitialization::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

#if AK_ENABLE_COMMUNICATION
	InitializationStructure.CommSettings.bInitSystemLib = InitializeSystemComms;
#endif
}

void FAkCommunicationSettingsWithCommSelection::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

#if AK_ENABLE_COMMUNICATION
	InitializationStructure.CommSettings.commSystem = (AkCommSettings::AkCommSystem)CommunicationSystem;
#endif
}

//////////////////////////////////////////////////////////////////////////
// FAkCommonInitializationSettings

void FAkCommonInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto& InitSettings = InitializationStructure.InitSettings;
	InitSettings.uMaxNumPaths = MaximumNumberOfPositioningPaths;
	InitSettings.uCommandQueueSize = CommandQueueSize;
	InitSettings.uNumSamplesPerFrame = SamplesPerFrame;

	MainOutputSettings.FillInitializationStructure(InitializationStructure);

	auto& PlatformInitSettings = InitializationStructure.PlatformInitSettings;
	PlatformInitSettings.uNumRefillsInVoice = NumberOfRefillsInVoice;

	SpatialAudioSettings.FillInitializationStructure(InitializationStructure);

	InitializationStructure.MusicSettings.fStreamingLookAheadRatio = StreamingLookAheadRatio;
}


//////////////////////////////////////////////////////////////////////////
// FAkAdvancedInitializationSettings

void FAkAdvancedInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto& DeviceSettings = InitializationStructure.DeviceSettings;
	DeviceSettings.uIOMemorySize = IO_MemorySize;
	DeviceSettings.uGranularity = IO_Granularity == 0 ? (32 * 1024) : IO_Granularity;
	DeviceSettings.fTargetAutoStmBufferLength = TargetAutoStreamBufferLength;
	DeviceSettings.bUseStreamCache = UseStreamCache;
	DeviceSettings.uMaxCachePinnedBytes = MaximumPinnedBytesInCache;

	auto& InitSettings = InitializationStructure.InitSettings;
	InitSettings.bEnableGameSyncPreparation = EnableGameSyncPreparation;
	InitSettings.uContinuousPlaybackLookAhead = ContinuousPlaybackLookAhead;
	InitSettings.uMonitorQueuePoolSize = MonitorQueuePoolSize;
	InitSettings.uMaxHardwareTimeoutMs = MaximumHardwareTimeoutMs;
	InitSettings.bDebugOutOfRangeCheckEnabled = DebugOutOfRangeCheckEnabled;
	InitSettings.fDebugOutOfRangeLimit = DebugOutOfRangeLimit;
}


//////////////////////////////////////////////////////////////////////////
// FAkAdvancedInitializationSettingsWithMultiCoreRendering

void FAkAdvancedInitializationSettingsWithMultiCoreRendering::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

	if (EnableMultiCoreRendering)
	{
		FAkAudioDevice* pDevice = FAkAudioDevice::Get();
		check(pDevice != nullptr);

		FAkJobWorkerScheduler* pScheduler = pDevice->GetAkJobWorkerScheduler();
		check(pScheduler != nullptr);

		auto& InitSettings = InitializationStructure.InitSettings;
		pScheduler->InstallJobWorkerScheduler(JobWorkerMaxExecutionTimeUSec, InitSettings.settingsJobManager);
	}
}

static void UELocalOutputFunc(
	AK::Monitor::ErrorCode in_eErrorCode,
	const AkOSChar* in_pszError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID)
{
	FString AkError(in_pszError);

	if (!IsRunningCommandlet())
	{
		if (in_eErrorLevel == AK::Monitor::ErrorLevel_Message)
		{
			UE_LOG(LogAkAudio, Log, TEXT("%s"), *AkError);
		}
		else
		{
			UE_LOG(LogAkAudio, Error, TEXT("%s"), *AkError);
		}
	}
}

namespace FAkSoundEngineInitialization
{
	bool Initialize(FWwiseIOHook* IOHook)
	{
		if (!IOHook)
		{
			UE_LOG(LogAkAudio, Error, TEXT("IOHook is null."));
			return false;
		}

		const UAkInitializationSettings* InitializationSettings = FAkPlatform::GetInitializationSettings();
		if (InitializationSettings == nullptr)
		{
			UE_LOG(LogAkAudio, Error, TEXT("InitializationSettings could not be found."));
			return false;
		}

		FAkInitializationStructure InitializationStructure;
		InitializationSettings->FillInitializationStructure(InitializationStructure);

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Platform"));
		FAkPlatform::PreInitialize(InitializationStructure);

		auto* Comm = FWwiseLowLevelComm::Get();
		auto* MemoryMgr = FWwiseLowLevelMemoryMgr::Get();
		auto* Monitor = FWwiseLowLevelMonitor::Get();
		auto* MusicEngine = FWwiseLowLevelMusicEngine::Get();
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		auto* StreamMgr = FWwiseLowLevelStreamMgr::Get();
		auto* GlobalCallbacks = FWwiseGlobalCallbacks::Get();

		// Enable AK error redirection to UE log.
		if (LIKELY(Monitor))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Monitor's Output"));
			Monitor->SetLocalOutput(AK::Monitor::ErrorLevel_All, UELocalOutputFunc);
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Memory Manager"));
		if (UNLIKELY(!MemoryMgr) || MemoryMgr->Init(&InitializationStructure.MemSettings) != AK_Success)
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize AK::MemoryMgr."));
			return false;
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Global Callbacks"));
		if (UNLIKELY(!GlobalCallbacks) || !GlobalCallbacks->Init())
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize Global Callbacks."));
			return false;
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Stream Manager"));
		if (UNLIKELY(!StreamMgr) || !StreamMgr->Create(InitializationStructure.StreamManagerSettings))
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize AK::StreamMgr."));
			return false;
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing IOHook"));
		if (!IOHook->Init(InitializationStructure.DeviceSettings))
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize IOHook."));
			return false;
		}

		if (AkInitializationSettings_Helpers::IsLoggingInitialization && InitializationStructure.InitSettings.szPluginDLLPath)
		{
			FString DllPath(InitializationStructure.InitSettings.szPluginDLLPath);
			UE_LOG(LogAkAudio, Log, TEXT("Wwise plug-in DLL path: %s"), *DllPath);
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Sound Engine"));
		if (UNLIKELY(!SoundEngine) || SoundEngine->Init(&InitializationStructure.InitSettings, &InitializationStructure.PlatformInitSettings) != AK_Success)
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize AK::SoundEngine."));
			return false;
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Music Engine"));
		if (UNLIKELY(!MusicEngine) || MusicEngine->Init(&InitializationStructure.MusicSettings) != AK_Success)
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize AK::MusicEngine."));
			return false;
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Spatial Audio"));
		if (UNLIKELY(!SpatialAudio) || SpatialAudio->Init(InitializationStructure.SpatialAudioInitSettings) != AK_Success)
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize AK::SpatialAudio."));
			return false;
		}

#if AK_ENABLE_COMMUNICATION
		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Communication"));
		if (UNLIKELY(!Comm) || Comm->Init(InitializationStructure.CommSettings) != AK_Success)
		{
			UE_LOG(LogAkAudio, Warning, TEXT("Could not initialize Wwise communication."));
		}
		else
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Log, TEXT("Wwise remote connection application name: %s"), ANSI_TO_TCHAR(InitializationStructure.CommSettings.szAppNetworkName));
		}
#endif

		return true;
	}

	void Finalize(FWwiseIOHook* IOHook)
	{
		auto* Comm = FWwiseLowLevelComm::Get();
		auto* MemoryMgr = FWwiseLowLevelMemoryMgr::Get();
		auto* Monitor = FWwiseLowLevelMonitor::Get();
		auto* MusicEngine = FWwiseLowLevelMusicEngine::Get();
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		auto* StreamMgr = FWwiseLowLevelStreamMgr::GetAkStreamMgr();
		auto* GlobalCallbacks = FWwiseGlobalCallbacks::Get();

#if AK_ENABLE_COMMUNICATION
		if (LIKELY(Comm))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Communication"));
			Comm->Term();
		}
#endif

		// Note: No Spatial Audio Term

		if (LIKELY(MusicEngine))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Music Engine"));
			MusicEngine->Term();
		}

		if (LIKELY(SoundEngine && SoundEngine->IsInitialized()))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Sound Engine"));
			SoundEngine->Term();
		}

		if (LIKELY(IOHook))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating IOHook"));
			IOHook->Term();
		}

		if (LIKELY(StreamMgr))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Stream Manager"));
			StreamMgr->Destroy();
		}

		if (LIKELY(GlobalCallbacks))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Global Callbacks"));
			GlobalCallbacks->Term();
		}

		if (LIKELY(MemoryMgr && MemoryMgr->IsInitialized()))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Memory Manager"));
			MemoryMgr->Term();
		}

		if (LIKELY(Monitor))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Resetting Monitor's Output"));
			Monitor->SetLocalOutput(0, nullptr);
		}
	}
}
