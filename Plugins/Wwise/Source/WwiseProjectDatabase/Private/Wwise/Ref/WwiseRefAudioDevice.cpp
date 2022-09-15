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

#include "Wwise/Ref/WwiseRefAudioDevice.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefAudioDevice::NAME = TEXT("AudioDevice");

const FWwiseMetadataPlugin* FWwiseRefAudioDevice::GetPlugin() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank || !SoundBank->Plugins))
	{
		return nullptr;
	}

	const auto& Plugins = SoundBank->Plugins->AudioDevices;
	if (Plugins.IsValidIndex(AudioDeviceIndex))
	{
		return &Plugins[AudioDeviceIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Audio Device index #%zu"), AudioDeviceIndex);
		return nullptr;
	}
}

uint32 FWwiseRefAudioDevice::AudioDeviceId() const
{
	const auto* AudioDevice = GetPlugin();
	if (UNLIKELY(!AudioDevice))
	{
		return 0;
	}
	return AudioDevice->Id;
}

FGuid FWwiseRefAudioDevice::AudioDeviceGuid() const
{
	const auto* AudioDevice = GetPlugin();
	if (UNLIKELY(!AudioDevice))
	{
		return {};
	}
	return AudioDevice->GUID;
}

FString FWwiseRefAudioDevice::AudioDeviceName() const
{
	const auto* AudioDevice = GetPlugin();
	if (UNLIKELY(!AudioDevice))
	{
		return {};
	}
	return AudioDevice->Name;
}

FString FWwiseRefAudioDevice::AudioDeviceObjectPath() const
{
	const auto* AudioDevice = GetPlugin();
	if (UNLIKELY(!AudioDevice))
	{
		return {};
	}
	return AudioDevice->ObjectPath;
}

uint32 FWwiseRefAudioDevice::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(AudioDeviceIndex));
	return Result;
}
