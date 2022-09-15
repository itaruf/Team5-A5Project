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

#include "Wwise/LowLevel/WwiseLowLevelStreamMgr.h"
#include "Wwise/WwiseSoundEngineModule.h"
#include "Wwise/Stats/SoundEngine.h"

AK::IAkStreamMgr* FWwiseLowLevelStreamMgr::GetAkStreamMgr()
{
	// Please don't move this implementation to the header file. It must be linked in this current linking unit.
	IWwiseSoundEngineModule::ForceLoadModule();
	return AK::IAkStreamMgr::Get();
}

AK::IAkStreamMgr* FWwiseLowLevelStreamMgr::Create(
	const AkStreamMgrSettings& in_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::StreamMgr::Create(in_settings);
}

void FWwiseLowLevelStreamMgr::GetDefaultSettings(
	AkStreamMgrSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::StreamMgr::GetDefaultSettings(out_settings);
}

AK::StreamMgr::IAkFileLocationResolver* FWwiseLowLevelStreamMgr::GetFileLocationResolver()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::StreamMgr::GetFileLocationResolver();
}

void FWwiseLowLevelStreamMgr::SetFileLocationResolver(
	AK::StreamMgr::IAkFileLocationResolver* in_pFileLocationResolver
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::StreamMgr::SetFileLocationResolver(in_pFileLocationResolver);
}

AkDeviceID FWwiseLowLevelStreamMgr::CreateDevice(
	const AkDeviceSettings& in_settings,
	AK::StreamMgr::IAkLowLevelIOHook* in_pLowLevelHook
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::StreamMgr::CreateDevice(in_settings, in_pLowLevelHook);
}

AKRESULT FWwiseLowLevelStreamMgr::DestroyDevice(
	AkDeviceID					in_deviceID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::StreamMgr::DestroyDevice(in_deviceID);
}

void FWwiseLowLevelStreamMgr::GetDefaultDeviceSettings(
	AkDeviceSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::StreamMgr::GetDefaultDeviceSettings(out_settings);
}

AKRESULT FWwiseLowLevelStreamMgr::SetCurrentLanguage(
	const AkOSChar* in_pszLanguageName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::StreamMgr::SetCurrentLanguage(in_pszLanguageName);
}

const AkOSChar* FWwiseLowLevelStreamMgr::GetCurrentLanguage()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::StreamMgr::GetCurrentLanguage();
}

AKRESULT FWwiseLowLevelStreamMgr::AddLanguageChangeObserver(
	AK::StreamMgr::AkLanguageChangeHandler in_handler,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::StreamMgr::AddLanguageChangeObserver(in_handler, in_pCookie);
}

void FWwiseLowLevelStreamMgr::RemoveLanguageChangeObserver(
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::StreamMgr::RemoveLanguageChangeObserver(in_pCookie);
}

void FWwiseLowLevelStreamMgr::FlushAllCaches()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::StreamMgr::FlushAllCaches();
}

