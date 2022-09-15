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


#include "Wwise/LowLevel/WwiseLowLevelMonitor.h"
#include "Wwise/Stats/SoundEngine.h"

#if WITH_EDITOR
#ifndef AK_OPTIMIZED
#if AK_SUPPORT_WAAPI
AkWAAPIErrorMessageTranslator FWwiseLowLevelMonitor::WaapiErrorMessageTranslator;
#endif //AK_SUPPORT_WAAPI
#endif //AK_OPTIMIZED
#endif //WITH_EDITOR


AKRESULT FWwiseLowLevelMonitor::PostCode(
	AK::Monitor::ErrorCode in_eError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID,
	AkUniqueID in_audioNodeID,
	bool in_bIsBus
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::Monitor::PostCode(in_eError, in_eErrorLevel, in_playingID, in_gameObjID, in_audioNodeID, in_bIsBus);
}

AKRESULT FWwiseLowLevelMonitor::PostCodeVarArg(
	AK::Monitor::ErrorCode in_eError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AK::Monitor::MsgContext msgContext,
	...
)
{
	va_list Args;
	va_start(Args, msgContext);
	auto Result = PostCodeVaList(in_eError, in_eErrorLevel, msgContext, Args);
	va_end(Args);
	return Result;
}

AKRESULT FWwiseLowLevelMonitor::PostCodeVaList(
	AK::Monitor::ErrorCode in_eError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AK::Monitor::MsgContext msgContext,
	::va_list args
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::Monitor::PostCodeVaList(in_eError, in_eErrorLevel, msgContext, args);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelMonitor::PostString(
	const wchar_t* in_pszError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID,
	AkUniqueID in_audioNodeID,
	bool in_bIsBus
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::Monitor::PostString(in_pszError, in_eErrorLevel, in_playingID, in_gameObjID, in_audioNodeID, in_bIsBus);
}
#endif // #ifdef AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelMonitor::PostString(
	const char* in_pszError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID,
	AkUniqueID in_audioNodeID,
	bool in_bIsBus
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::Monitor::PostString(in_pszError, in_eErrorLevel, in_playingID, in_gameObjID, in_audioNodeID, in_bIsBus);
}

AKRESULT FWwiseLowLevelMonitor::SetLocalOutput(
	AkUInt32 in_uErrorLevel,
	AK::Monitor::LocalOutputFunc in_pMonitorFunc
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::Monitor::SetLocalOutput(in_uErrorLevel, in_pMonitorFunc);
}

AKRESULT FWwiseLowLevelMonitor::AddTranslator(
	AkErrorMessageTranslator* translator,
	bool overridePreviousTranslators
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::Monitor::AddTranslator(translator, overridePreviousTranslators);
}

AKRESULT FWwiseLowLevelMonitor::ResetTranslator(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::Monitor::ResetTranslator();
}

AkTimeMs FWwiseLowLevelMonitor::GetTimeStamp()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::Monitor::GetTimeStamp();
}

void FWwiseLowLevelMonitor::MonitorStreamMgrInit(
	const AkStreamMgrSettings& in_streamMgrSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::Monitor::MonitorStreamMgrInit(in_streamMgrSettings);
}

void FWwiseLowLevelMonitor::MonitorStreamingDeviceInit(
	AkDeviceID in_deviceID,
	const AkDeviceSettings& in_deviceSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::Monitor::MonitorStreamingDeviceInit(in_deviceID, in_deviceSettings);
}

void FWwiseLowLevelMonitor::MonitorStreamingDeviceDestroyed(
	AkDeviceID in_deviceID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::Monitor::MonitorStreamingDeviceDestroyed(in_deviceID);
}

void FWwiseLowLevelMonitor::MonitorStreamMgrTerm()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::Monitor::MonitorStreamMgrTerm();
}

void FWwiseLowLevelMonitor::SetupDefaultWAAPIErrorTranslator(const FString& WaapiIP, AkUInt32 WaapiPort, AkUInt32 Timeout)
{
#if WITH_EDITOR
#ifndef AK_OPTIMIZED
#if AK_SUPPORT_WAAPI
	WaapiErrorMessageTranslator.SetConnectionIP(TCHAR_TO_ANSI(*WaapiIP), WaapiPort, Timeout);
	AddTranslator(&WaapiErrorMessageTranslator);
#endif //AK_SUPPORT_WAAPI
#endif //AK_OPTIMIZED
#endif //WITH_EDITOR
}

void FWwiseLowLevelMonitor::TerminateDefaultWAAPIErrorTranslator()
{
#if WITH_EDITOR
#ifndef AK_OPTIMIZED
#if AK_SUPPORT_WAAPI
	WaapiErrorMessageTranslator.Term();
#endif //AK_SUPPORT_WAAPI
#endif //AK_OPTIMIZED
#endif //WITH_EDITOR
}

