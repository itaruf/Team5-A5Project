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


#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"
#include "Wwise/Stats/SoundEngine.h"

#include <AK/Plugin/AkVorbisDecoderFactory.h>
#include <AK/Plugin/AkMeterFXFactory.h>
#include <AK/Plugin/AkAudioInputSourceFactory.h>

#if AK_SUPPORT_OPUS
#include <AK/Plugin/AkOpusDecoderFactory.h>
#endif // AK_SUPPORT_OPUS

#if PLATFORM_IOS && !PLATFORM_TVOS
#include "Generated/AkiOSPlugins.h"
#endif

#if PLATFORM_SWITCH
#include "Generated/AkSwitchPlugins.h"
#if AK_SUPPORT_OPUS
#include <AK/Plugin/AkOpusNXFactory.h>
#endif
#endif

#if PLATFORM_TVOS
#include "Generated/AkTVOSPlugins.h"
#endif

bool FWwiseLowLevelSoundEngine::IsInitialized()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::IsInitialized();
}

AKRESULT FWwiseLowLevelSoundEngine::Init(
	AkInitSettings* in_pSettings,
	AkPlatformInitSettings* in_pPlatformSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Init(in_pSettings, in_pPlatformSettings);
}

void FWwiseLowLevelSoundEngine::GetDefaultInitSettings(
	AkInitSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::GetDefaultInitSettings(out_settings);
}

void FWwiseLowLevelSoundEngine::GetDefaultPlatformInitSettings(
	AkPlatformInitSettings& out_platformSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::GetDefaultPlatformInitSettings(out_platformSettings);
}

void FWwiseLowLevelSoundEngine::Term()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::Term();
}

AKRESULT FWwiseLowLevelSoundEngine::GetAudioSettings(
	AkAudioSettings& out_audioSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetAudioSettings(out_audioSettings);
}

AkChannelConfig FWwiseLowLevelSoundEngine::GetSpeakerConfiguration(
	AkOutputDeviceID	in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetSpeakerConfiguration(in_idOutput);
}

AKRESULT FWwiseLowLevelSoundEngine::GetOutputDeviceConfiguration(
	AkOutputDeviceID in_idOutput,
	AkChannelConfig& io_channelConfig,
	Ak3DAudioSinkCapabilities& io_capabilities
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetOutputDeviceConfiguration(in_idOutput, io_channelConfig, io_capabilities);
}

AKRESULT FWwiseLowLevelSoundEngine::GetPanningRule(
	AkPanningRule& out_ePanningRule,
	AkOutputDeviceID	in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetPanningRule(out_ePanningRule, in_idOutput);
}

AKRESULT FWwiseLowLevelSoundEngine::SetPanningRule(
	AkPanningRule		in_ePanningRule,
	AkOutputDeviceID	in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetPanningRule(in_ePanningRule, in_idOutput);
}

AKRESULT FWwiseLowLevelSoundEngine::GetSpeakerAngles(
	AkReal32* io_pfSpeakerAngles,
	AkUInt32& io_uNumAngles,
	AkReal32& out_fHeightAngle,
	AkOutputDeviceID	in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetSpeakerAngles(io_pfSpeakerAngles, io_uNumAngles, out_fHeightAngle, in_idOutput);
}

AKRESULT FWwiseLowLevelSoundEngine::SetSpeakerAngles(
	const AkReal32* in_pfSpeakerAngles,
	AkUInt32			in_uNumAngles,
	AkReal32 			in_fHeightAngle,
	AkOutputDeviceID	in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetSpeakerAngles(in_pfSpeakerAngles, in_uNumAngles, in_fHeightAngle, in_idOutput);
}

AKRESULT FWwiseLowLevelSoundEngine::SetVolumeThreshold(
	AkReal32 in_fVolumeThresholdDB
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetVolumeThreshold(in_fVolumeThresholdDB);
}

AKRESULT FWwiseLowLevelSoundEngine::SetMaxNumVoicesLimit(
	AkUInt16 in_maxNumberVoices
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetMaxNumVoicesLimit(in_maxNumberVoices);
}

AKRESULT FWwiseLowLevelSoundEngine::RenderAudio(
	bool in_bAllowSyncRender
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RenderAudio(in_bAllowSyncRender);
}

AK::IAkGlobalPluginContext* FWwiseLowLevelSoundEngine::GetGlobalPluginContext()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetGlobalPluginContext();
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterPlugin(
	AkPluginType in_eType,
	AkUInt32 in_ulCompanyID,
	AkUInt32 in_ulPluginID,
	AkCreatePluginCallback in_pCreateFunc,
	AkCreateParamCallback in_pCreateParamFunc,
	AkGetDeviceListCallback in_pGetDeviceList
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterPlugin(in_eType, in_ulCompanyID, in_ulPluginID, in_pCreateFunc, in_pCreateParamFunc, in_pGetDeviceList);
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterPluginDLL(
	const AkOSChar* in_DllName,
	const AkOSChar* in_DllPath
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterPluginDLL(in_DllName, in_DllPath);
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterCodec(
	AkUInt32 in_ulCompanyID,
	AkUInt32 in_ulCodecID,
	AkCreateFileSourceCallback in_pFileCreateFunc,
	AkCreateBankSourceCallback in_pBankCreateFunc
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterCodec(in_ulCompanyID, in_ulCodecID, in_pFileCreateFunc, in_pBankCreateFunc);
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterGlobalCallback(
	AkGlobalCallbackFunc in_pCallback,
	AkUInt32 in_eLocation,
	void* in_pCookie,
	AkPluginType in_eType,
	AkUInt32 in_ulCompanyID,
	AkUInt32 in_ulPluginID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterGlobalCallback(in_pCallback, in_eLocation, in_pCookie, in_eType, in_ulCompanyID, in_ulPluginID);
}

AKRESULT FWwiseLowLevelSoundEngine::UnregisterGlobalCallback(
	AkGlobalCallbackFunc in_pCallback,
	AkUInt32 in_eLocation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnregisterGlobalCallback(in_pCallback, in_eLocation);
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterResourceMonitorCallback(
	AkResourceMonitorCallbackFunc in_pCallback
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterResourceMonitorCallback(in_pCallback);
}

AKRESULT FWwiseLowLevelSoundEngine::UnregisterResourceMonitorCallback(
	AkResourceMonitorCallbackFunc in_pCallback
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnregisterResourceMonitorCallback(in_pCallback);
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterAudioDeviceStatusCallback(
	AK::AkDeviceStatusCallbackFunc in_pCallback
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterAudioDeviceStatusCallback(in_pCallback);
}

AKRESULT FWwiseLowLevelSoundEngine::UnregisterAudioDeviceStatusCallback()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnregisterAudioDeviceStatusCallback();
}

#ifdef AK_SUPPORT_WCHAR
AkUInt32 FWwiseLowLevelSoundEngine::GetIDFromString(const wchar_t* in_pszString)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetIDFromString(in_pszString);
}
#endif //AK_SUPPORT_WCHAR

AkUInt32 FWwiseLowLevelSoundEngine::GetIDFromString(const char* in_pszString)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetIDFromString(in_pszString);
}

AkPlayingID FWwiseLowLevelSoundEngine::PostEvent(
	AkUniqueID in_eventID,
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_uFlags,
	AkCallbackFunc in_pfnCallback,
	void* in_pCookie,
	AkUInt32 in_cExternals,
	AkExternalSourceInfo* in_pExternalSources,
	AkPlayingID	in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PostEvent(in_eventID, in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID);
}

#ifdef AK_SUPPORT_WCHAR
AkPlayingID FWwiseLowLevelSoundEngine::PostEvent(
	const wchar_t* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_uFlags,
	AkCallbackFunc in_pfnCallback,
	void* in_pCookie,
	AkUInt32 in_cExternals,
	AkExternalSourceInfo* in_pExternalSources,
	AkPlayingID	in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PostEvent(in_pszEventName, in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID);
}
#endif //AK_SUPPORT_WCHAR

AkPlayingID FWwiseLowLevelSoundEngine::PostEvent(
	const char* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_uFlags,
	AkCallbackFunc in_pfnCallback,
	void* in_pCookie,
	AkUInt32 in_cExternals,
	AkExternalSourceInfo* in_pExternalSources,
	AkPlayingID	in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PostEvent(in_pszEventName, in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID);
}

AKRESULT FWwiseLowLevelSoundEngine::ExecuteActionOnEvent(
	AkUniqueID in_eventID,
	AK::SoundEngine::AkActionOnEventType in_ActionType,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::ExecuteActionOnEvent(in_eventID, in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::ExecuteActionOnEvent(
	const wchar_t* in_pszEventName,
	AK::SoundEngine::AkActionOnEventType in_ActionType,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::ExecuteActionOnEvent(in_pszEventName, in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::ExecuteActionOnEvent(
	const char* in_pszEventName,
	AK::SoundEngine::AkActionOnEventType in_ActionType,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::ExecuteActionOnEvent(in_pszEventName, in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID);
}

AkPlayingID FWwiseLowLevelSoundEngine::PostMIDIOnEvent(
	AkUniqueID in_eventID,
	AkGameObjectID in_gameObjectID,
	AkMIDIPost* in_pPosts,
	AkUInt16 in_uNumPosts,
	bool in_bAbsoluteOffsets,
	AkUInt32 in_uFlags,
	AkCallbackFunc in_pfnCallback,
	void* in_pCookie,
	AkPlayingID in_playingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PostMIDIOnEvent(in_eventID, in_gameObjectID, in_pPosts, in_uNumPosts, in_bAbsoluteOffsets, in_uFlags, in_pfnCallback, in_pCookie, in_playingID);
}

AKRESULT FWwiseLowLevelSoundEngine::StopMIDIOnEvent(
	AkUniqueID in_eventID,
	AkGameObjectID in_gameObjectID,
	AkPlayingID in_playingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::StopMIDIOnEvent(in_eventID, in_gameObjectID, in_playingID);
}

AKRESULT FWwiseLowLevelSoundEngine::PinEventInStreamCache(
	AkUniqueID in_eventID,
	AkPriority in_uActivePriority,
	AkPriority in_uInactivePriority
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PinEventInStreamCache(in_eventID, in_uActivePriority, in_uInactivePriority);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::PinEventInStreamCache(
	const wchar_t* in_pszEventName,
	AkPriority in_uActivePriority,
	AkPriority in_uInactivePriority
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PinEventInStreamCache(in_pszEventName, in_uActivePriority, in_uInactivePriority);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::PinEventInStreamCache(
	const char* in_pszEventName,
	AkPriority in_uActivePriority,
	AkPriority in_uInactivePriority
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PinEventInStreamCache(in_pszEventName, in_uActivePriority, in_uInactivePriority);
}

AKRESULT FWwiseLowLevelSoundEngine::UnpinEventInStreamCache(
	AkUniqueID in_eventID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnpinEventInStreamCache(in_eventID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::UnpinEventInStreamCache(
	const wchar_t* in_pszEventName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnpinEventInStreamCache(in_pszEventName);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::UnpinEventInStreamCache(
	const char* in_pszEventName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnpinEventInStreamCache(in_pszEventName);
}

AKRESULT FWwiseLowLevelSoundEngine::GetBufferStatusForPinnedEvent(
	AkUniqueID in_eventID,
	AkReal32& out_fPercentBuffered,
	bool& out_bCachePinnedMemoryFull
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetBufferStatusForPinnedEvent(in_eventID, out_fPercentBuffered, out_bCachePinnedMemoryFull);
}

AKRESULT FWwiseLowLevelSoundEngine::GetBufferStatusForPinnedEvent(
	const char* in_pszEventName,
	AkReal32& out_fPercentBuffered,
	bool& out_bCachePinnedMemoryFull
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetBufferStatusForPinnedEvent(in_pszEventName, out_fPercentBuffered, out_bCachePinnedMemoryFull);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::GetBufferStatusForPinnedEvent(
	const wchar_t* in_pszEventName,
	AkReal32& out_fPercentBuffered,
	bool& out_bCachePinnedMemoryFull
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetBufferStatusForPinnedEvent(in_pszEventName, out_fPercentBuffered, out_bCachePinnedMemoryFull);
}
#endif

AKRESULT FWwiseLowLevelSoundEngine::SeekOnEvent(
	AkUniqueID in_eventID,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_iPosition,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SeekOnEvent(in_eventID, in_gameObjectID, in_iPosition, in_bSeekToNearestMarker, in_PlayingID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::SeekOnEvent(
	const wchar_t* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_iPosition,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SeekOnEvent(in_pszEventName, in_gameObjectID, in_iPosition, in_bSeekToNearestMarker, in_PlayingID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::SeekOnEvent(
	const char* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_iPosition,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SeekOnEvent(in_pszEventName, in_gameObjectID, in_iPosition, in_bSeekToNearestMarker, in_PlayingID);
}

AKRESULT FWwiseLowLevelSoundEngine::SeekOnEvent(
	AkUniqueID in_eventID,
	AkGameObjectID in_gameObjectID,
	AkReal32 in_fPercent,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SeekOnEvent(in_eventID, in_gameObjectID, in_fPercent, in_bSeekToNearestMarker, in_PlayingID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::SeekOnEvent(
	const wchar_t* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkReal32 in_fPercent,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SeekOnEvent(in_pszEventName, in_gameObjectID, in_fPercent, in_bSeekToNearestMarker, in_PlayingID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::SeekOnEvent(
	const char* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkReal32 in_fPercent,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SeekOnEvent(in_pszEventName, in_gameObjectID, in_fPercent, in_bSeekToNearestMarker, in_PlayingID);
}

void FWwiseLowLevelSoundEngine::CancelEventCallbackCookie(
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::CancelEventCallbackCookie(in_pCookie);
}

void FWwiseLowLevelSoundEngine::CancelEventCallbackGameObject(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::CancelEventCallbackGameObject(in_gameObjectID);
}

void FWwiseLowLevelSoundEngine::CancelEventCallback(
	AkPlayingID in_playingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::CancelEventCallback(in_playingID);
}

AKRESULT FWwiseLowLevelSoundEngine::GetSourcePlayPosition(
	AkPlayingID		in_PlayingID,
	AkTimeMs* out_puPosition,
	bool			in_bExtrapolate
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetSourcePlayPosition(in_PlayingID, out_puPosition, in_bExtrapolate);
}

AKRESULT FWwiseLowLevelSoundEngine::GetSourcePlayPositions(
	AkPlayingID		in_PlayingID,
	AkSourcePosition* out_puPositions,
	AkUInt32* io_pcPositions,
	bool			in_bExtrapolate
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetSourcePlayPositions(in_PlayingID, out_puPositions, io_pcPositions, in_bExtrapolate);
}

AKRESULT FWwiseLowLevelSoundEngine::GetSourceStreamBuffering(
	AkPlayingID		in_PlayingID,
	AkTimeMs& out_buffering,
	bool& out_bIsBuffering
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetSourceStreamBuffering(in_PlayingID, out_buffering, out_bIsBuffering);
}

void FWwiseLowLevelSoundEngine::StopAll(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::StopAll(in_gameObjectID);
}

void FWwiseLowLevelSoundEngine::StopPlayingID(
	AkPlayingID in_playingID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::StopPlayingID(in_playingID, in_uTransitionDuration, in_eFadeCurve);
}

void FWwiseLowLevelSoundEngine::ExecuteActionOnPlayingID(
	AK::SoundEngine::AkActionOnEventType in_ActionType,
	AkPlayingID in_playingID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::ExecuteActionOnPlayingID(in_ActionType, in_playingID, in_uTransitionDuration, in_eFadeCurve);
}

void FWwiseLowLevelSoundEngine::SetRandomSeed(
	AkUInt32 in_uSeed
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::SetRandomSeed(in_uSeed);
}

void FWwiseLowLevelSoundEngine::MuteBackgroundMusic(
	bool in_bMute
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SoundEngine::MuteBackgroundMusic(in_bMute);
}

bool FWwiseLowLevelSoundEngine::GetBackgroundMusicMute()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetBackgroundMusicMute();
}

AKRESULT FWwiseLowLevelSoundEngine::SendPluginCustomGameData(
	AkUniqueID in_busID,
	AkGameObjectID in_busObjectID,
	AkPluginType in_eType,
	AkUInt32 in_uCompanyID,
	AkUInt32 in_uPluginID,
	const void* in_pData,
	AkUInt32 in_uSizeInBytes
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SendPluginCustomGameData(in_busID, in_busObjectID, in_eType, in_uCompanyID, in_uPluginID, in_pData, in_uSizeInBytes);
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterGameObj(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterGameObj(in_gameObjectID);
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterGameObj(
	AkGameObjectID in_gameObjectID,
	const char* in_pszObjName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterGameObj(in_gameObjectID, in_pszObjName);
}

AKRESULT FWwiseLowLevelSoundEngine::UnregisterGameObj(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnregisterGameObj(in_gameObjectID);
}

AKRESULT FWwiseLowLevelSoundEngine::UnregisterAllGameObj(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnregisterAllGameObj();
}

AKRESULT FWwiseLowLevelSoundEngine::SetPosition(
	AkGameObjectID in_GameObjectID,
	const AkSoundPosition& in_Position,
	AkSetPositionFlags in_eFlags
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetPosition(in_GameObjectID, in_Position, in_eFlags);
}

AKRESULT FWwiseLowLevelSoundEngine::SetMultiplePositions(
	AkGameObjectID in_GameObjectID,
	const AkSoundPosition* in_pPositions,
	AkUInt16 in_NumPositions,
	AK::SoundEngine::MultiPositionType in_eMultiPositionType,
	AkSetPositionFlags in_eFlags
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetMultiplePositions(in_GameObjectID, in_pPositions, in_NumPositions, in_eMultiPositionType, in_eFlags);
}

AKRESULT FWwiseLowLevelSoundEngine::SetMultiplePositions(
	AkGameObjectID in_GameObjectID,
	const AkChannelEmitter* in_pPositions,
	AkUInt16 in_NumPositions,
	AK::SoundEngine::MultiPositionType in_eMultiPositionType,
	AkSetPositionFlags in_eFlags
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetMultiplePositions(in_GameObjectID, in_pPositions, in_NumPositions, in_eMultiPositionType, in_eFlags);
}

AKRESULT FWwiseLowLevelSoundEngine::SetScalingFactor(
	AkGameObjectID in_GameObjectID,
	AkReal32 in_fAttenuationScalingFactor
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetScalingFactor(in_GameObjectID, in_fAttenuationScalingFactor);
}

AKRESULT FWwiseLowLevelSoundEngine::SetDistanceProbe(
	AkGameObjectID in_listenerGameObjectID,
	AkGameObjectID in_distanceProbeGameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetDistanceProbe(in_listenerGameObjectID, in_distanceProbeGameObjectID);
}

AKRESULT FWwiseLowLevelSoundEngine::ClearBanks()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::ClearBanks();
}

AKRESULT FWwiseLowLevelSoundEngine::SetBankLoadIOSettings(
	AkReal32            in_fThroughput,
	AkPriority          in_priority
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetBankLoadIOSettings(in_fThroughput, in_priority);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::LoadBank(
	const wchar_t* in_pszString,
	AkBankID& out_bankID,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::LoadBank(in_pszString, out_bankID, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::LoadBank(
	const char* in_pszString,
	AkBankID& out_bankID,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::LoadBank(in_pszString, out_bankID, in_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::LoadBank(
	AkBankID			in_bankID,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::LoadBank(in_bankID, in_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::LoadBankMemoryView(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkBankID& out_bankID,
	AkBankType& out_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::LoadBankMemoryView(in_pInMemoryBankPtr, in_uInMemoryBankSize, out_bankID, out_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::LoadBankMemoryCopy(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkBankID& out_bankID,
	AkBankType& out_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::LoadBankMemoryCopy(in_pInMemoryBankPtr, in_uInMemoryBankSize, out_bankID, out_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::DecodeBank(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkMemPoolId			in_uPoolForDecodedBank,
	void*& out_pDecodedBankPtr,
	AkUInt32& out_uDecodedBankSize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::DecodeBank(in_pInMemoryBankPtr, in_uInMemoryBankSize, in_uPoolForDecodedBank, out_pDecodedBankPtr, out_uDecodedBankSize);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::LoadBank(
	const wchar_t* in_pszString,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankID& out_bankID,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::LoadBank(in_pszString, in_pfnBankCallback, in_pCookie, out_bankID, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::LoadBank(
	const char* in_pszString,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankID& out_bankID,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::LoadBank(in_pszString, in_pfnBankCallback, in_pCookie, out_bankID, in_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::LoadBank(
	AkBankID			in_bankID,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::LoadBank(in_bankID, in_pfnBankCallback, in_pCookie, in_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::LoadBankMemoryView(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankID& out_bankID,
	AkBankType& out_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::LoadBankMemoryView(in_pInMemoryBankPtr, in_uInMemoryBankSize, in_pfnBankCallback, in_pCookie, out_bankID, out_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::LoadBankMemoryCopy(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankID& out_bankID,
	AkBankType& out_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::LoadBankMemoryCopy(in_pInMemoryBankPtr, in_uInMemoryBankSize, in_pfnBankCallback, in_pCookie, out_bankID, out_bankType);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::UnloadBank(
	const wchar_t* in_pszString,
	const void* in_pInMemoryBankPtr,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnloadBank(in_pszString, in_pInMemoryBankPtr, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::UnloadBank(
	const char* in_pszString,
	const void* in_pInMemoryBankPtr,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnloadBank(in_pszString, in_pInMemoryBankPtr, in_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::UnloadBank(
	AkBankID            in_bankID,
	const void* in_pInMemoryBankPtr,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnloadBank(in_bankID, in_pInMemoryBankPtr, in_bankType);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::UnloadBank(
	const wchar_t* in_pszString,
	const void* in_pInMemoryBankPtr,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnloadBank(in_pszString, in_pInMemoryBankPtr, in_pfnBankCallback, in_pCookie, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::UnloadBank(
	const char* in_pszString,
	const void* in_pInMemoryBankPtr,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnloadBank(in_pszString, in_pInMemoryBankPtr, in_pfnBankCallback, in_pCookie, in_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::UnloadBank(
	AkBankID            in_bankID,
	const void* in_pInMemoryBankPtr,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnloadBank(in_bankID, in_pInMemoryBankPtr, in_pfnBankCallback, in_pCookie, in_bankType);
}

void FWwiseLowLevelSoundEngine::CancelBankCallbackCookie(
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::CancelBankCallbackCookie(in_pCookie);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::PrepareBank(
	AK::SoundEngine::PreparationType	in_PreparationType,
	const wchar_t* in_pszString,
	AK::SoundEngine::AkBankContent	in_uFlags,
	AkBankType						in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_pszString, in_uFlags, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::PrepareBank(
	AK::SoundEngine::PreparationType	in_PreparationType,
	const char* in_pszString,
	AK::SoundEngine::AkBankContent	in_uFlags,
	AkBankType						in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_pszString, in_uFlags, in_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::PrepareBank(
	AK::SoundEngine::PreparationType	in_PreparationType,
	AkBankID							in_bankID,
	AK::SoundEngine::AkBankContent		in_uFlags,
	AkBankType							in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_bankID, in_uFlags, in_bankType);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::PrepareBank(
	AK::SoundEngine::PreparationType	in_PreparationType,
	const wchar_t* in_pszString,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie,
	AK::SoundEngine::AkBankContent	in_uFlags,
	AkBankType						in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_pszString, in_pfnBankCallback, in_pCookie, in_uFlags, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::PrepareBank(
	AK::SoundEngine::PreparationType	in_PreparationType,
	const char* in_pszString,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie,
	AK::SoundEngine::AkBankContent	in_uFlags,
	AkBankType						in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_pszString, in_pfnBankCallback, in_pCookie, in_uFlags, in_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::PrepareBank(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkBankID			in_bankID,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie,
	AK::SoundEngine::AkBankContent	in_uFlags,
	AkBankType						in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_bankID, in_pfnBankCallback, in_pCookie, in_uFlags, in_bankType);
}

AKRESULT FWwiseLowLevelSoundEngine::ClearPreparedEvents()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::ClearPreparedEvents();
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	const wchar_t** in_ppszString,
	AkUInt32			in_uNumEvent
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_ppszString, in_uNumEvent);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	const char** in_ppszString,
	AkUInt32			in_uNumEvent
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_ppszString, in_uNumEvent);
}

AKRESULT FWwiseLowLevelSoundEngine::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkUniqueID* in_pEventID,
	AkUInt32			in_uNumEvent
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_pEventID, in_uNumEvent);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	const wchar_t** in_ppszString,
	AkUInt32			in_uNumEvent,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_ppszString, in_uNumEvent, in_pfnBankCallback, in_pCookie);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	const char** in_ppszString,
	AkUInt32			in_uNumEvent,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_ppszString, in_uNumEvent, in_pfnBankCallback, in_pCookie);
}

AKRESULT FWwiseLowLevelSoundEngine::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkUniqueID* in_pEventID,
	AkUInt32			in_uNumEvent,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_pEventID, in_uNumEvent, in_pfnBankCallback, in_pCookie);
}

AKRESULT FWwiseLowLevelSoundEngine::SetMedia(
	AkSourceSettings* in_pSourceSettings,
	AkUInt32			in_uNumSourceSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetMedia(in_pSourceSettings, in_uNumSourceSettings);
}

AKRESULT FWwiseLowLevelSoundEngine::UnsetMedia(
	AkSourceSettings* in_pSourceSettings,
	AkUInt32			in_uNumSourceSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnsetMedia(in_pSourceSettings, in_uNumSourceSettings);
}

AKRESULT FWwiseLowLevelSoundEngine::TryUnsetMedia(
	AkSourceSettings* in_pSourceSettings,
	AkUInt32          in_uNumSourceSettings,
	AKRESULT* out_pUnsetResults
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::TryUnsetMedia(in_pSourceSettings, in_uNumSourceSettings, out_pUnsetResults);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::PrepareGameSyncs(
	AK::SoundEngine::PreparationType	in_PreparationType,
	AkGroupType		in_eGameSyncType,
	const wchar_t* in_pszGroupName,
	const wchar_t** in_ppszGameSyncName,
	AkUInt32		in_uNumGameSyncs
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_pszGroupName, in_ppszGameSyncName, in_uNumGameSyncs);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::PrepareGameSyncs(
	AK::SoundEngine::PreparationType	in_PreparationType,
	AkGroupType		in_eGameSyncType,
	const char* in_pszGroupName,
	const char** in_ppszGameSyncName,
	AkUInt32		in_uNumGameSyncs
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_pszGroupName, in_ppszGameSyncName, in_uNumGameSyncs);
}

AKRESULT FWwiseLowLevelSoundEngine::PrepareGameSyncs(
	AK::SoundEngine::PreparationType	in_PreparationType,
	AkGroupType		in_eGameSyncType,
	AkUInt32		in_GroupID,
	AkUInt32* in_paGameSyncID,
	AkUInt32		in_uNumGameSyncs
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_GroupID, in_paGameSyncID, in_uNumGameSyncs);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::PrepareGameSyncs(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkGroupType			in_eGameSyncType,
	const wchar_t* in_pszGroupName,
	const wchar_t** in_ppszGameSyncName,
	AkUInt32			in_uNumGameSyncs,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_pszGroupName, in_ppszGameSyncName, in_uNumGameSyncs, in_pfnBankCallback, in_pCookie);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::PrepareGameSyncs(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkGroupType			in_eGameSyncType,
	const char* in_pszGroupName,
	const char** in_ppszGameSyncName,
	AkUInt32			in_uNumGameSyncs,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_pszGroupName, in_ppszGameSyncName, in_uNumGameSyncs, in_pfnBankCallback, in_pCookie);
}

AKRESULT FWwiseLowLevelSoundEngine::PrepareGameSyncs(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkGroupType			in_eGameSyncType,
	AkUInt32			in_GroupID,
	AkUInt32* in_paGameSyncID,
	AkUInt32			in_uNumGameSyncs,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_GroupID, in_paGameSyncID, in_uNumGameSyncs, in_pfnBankCallback, in_pCookie);
}

AKRESULT FWwiseLowLevelSoundEngine::SetListeners(
	AkGameObjectID in_emitterGameObj,
	const AkGameObjectID* in_pListenerGameObjs,
	AkUInt32 in_uNumListeners
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetListeners(in_emitterGameObj, in_pListenerGameObjs, in_uNumListeners);
}

AKRESULT FWwiseLowLevelSoundEngine::AddListener(
	AkGameObjectID in_emitterGameObj,
	AkGameObjectID in_listenerGameObj
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::AddListener(in_emitterGameObj, in_listenerGameObj);
}

AKRESULT FWwiseLowLevelSoundEngine::RemoveListener(
	AkGameObjectID in_emitterGameObj,
	AkGameObjectID in_listenerGameObj
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RemoveListener(in_emitterGameObj, in_listenerGameObj);
}

AKRESULT FWwiseLowLevelSoundEngine::SetDefaultListeners(
	const AkGameObjectID* in_pListenerObjs,
	AkUInt32 in_uNumListeners
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetDefaultListeners(in_pListenerObjs, in_uNumListeners);
}

AKRESULT FWwiseLowLevelSoundEngine::AddDefaultListener(
	AkGameObjectID in_listenerGameObj
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::AddDefaultListener(in_listenerGameObj);
}

AKRESULT FWwiseLowLevelSoundEngine::RemoveDefaultListener(
	AkGameObjectID in_listenerGameObj
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RemoveDefaultListener(in_listenerGameObj);
}

AKRESULT FWwiseLowLevelSoundEngine::ResetListenersToDefault(
	AkGameObjectID in_emitterGameObj
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::ResetListenersToDefault(in_emitterGameObj);
}

AKRESULT FWwiseLowLevelSoundEngine::SetListenerSpatialization(
	AkGameObjectID in_uListenerID,
	bool in_bSpatialized,
	AkChannelConfig in_channelConfig,
	AK::SpeakerVolumes::VectorPtr in_pVolumeOffsets
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetListenerSpatialization(in_uListenerID, in_bSpatialized, in_channelConfig, in_pVolumeOffsets);
}

AKRESULT FWwiseLowLevelSoundEngine::SetRTPCValue(
	AkRtpcID in_rtpcID,
	AkRtpcValue in_value,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetRTPCValue(in_rtpcID, in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::SetRTPCValue(
	const wchar_t* in_pszRtpcName,
	AkRtpcValue in_value,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetRTPCValue(in_pszRtpcName, in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::SetRTPCValue(
	const char* in_pszRtpcName,
	AkRtpcValue in_value,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetRTPCValue(in_pszRtpcName, in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

AKRESULT FWwiseLowLevelSoundEngine::SetRTPCValueByPlayingID(
	AkRtpcID in_rtpcID,
	AkRtpcValue in_value,
	AkPlayingID in_playingID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetRTPCValueByPlayingID(in_rtpcID, in_value, in_playingID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::SetRTPCValueByPlayingID(
	const wchar_t* in_pszRtpcName,
	AkRtpcValue in_value,
	AkPlayingID in_playingID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetRTPCValueByPlayingID(in_pszRtpcName, in_value, in_playingID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::SetRTPCValueByPlayingID(
	const char* in_pszRtpcName,
	AkRtpcValue in_value,
	AkPlayingID in_playingID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetRTPCValueByPlayingID(in_pszRtpcName, in_value, in_playingID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

AKRESULT FWwiseLowLevelSoundEngine::ResetRTPCValue(
	AkRtpcID in_rtpcID,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::ResetRTPCValue(in_rtpcID, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::ResetRTPCValue(
	const wchar_t* in_pszRtpcName,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::ResetRTPCValue(in_pszRtpcName, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::ResetRTPCValue(
	const char* in_pszRtpcName,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::ResetRTPCValue(in_pszRtpcName, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

AKRESULT FWwiseLowLevelSoundEngine::SetSwitch(
	AkSwitchGroupID in_switchGroup,
	AkSwitchStateID in_switchState,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetSwitch(in_switchGroup, in_switchState, in_gameObjectID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::SetSwitch(
	const wchar_t* in_pszSwitchGroup,
	const wchar_t* in_pszSwitchState,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetSwitch(in_pszSwitchGroup, in_pszSwitchState, in_gameObjectID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::SetSwitch(
	const char* in_pszSwitchGroup,
	const char* in_pszSwitchState,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetSwitch(in_pszSwitchGroup, in_pszSwitchState, in_gameObjectID);
}

AKRESULT FWwiseLowLevelSoundEngine::PostTrigger(
	AkTriggerID 	in_triggerID,
	AkGameObjectID 	in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PostTrigger(in_triggerID, in_gameObjectID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::PostTrigger(
	const wchar_t* in_pszTrigger,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PostTrigger(in_pszTrigger, in_gameObjectID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::PostTrigger(
	const char* in_pszTrigger,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::PostTrigger(in_pszTrigger, in_gameObjectID);
}

AKRESULT FWwiseLowLevelSoundEngine::SetState(
	AkStateGroupID in_stateGroup,
	AkStateID in_state
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetState(in_stateGroup, in_state);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::SetState(
	const wchar_t* in_pszStateGroup,
	const wchar_t* in_pszState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetState(in_pszStateGroup, in_pszState);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::SetState(
	const char* in_pszStateGroup,
	const char* in_pszState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetState(in_pszStateGroup, in_pszState);
}

AKRESULT FWwiseLowLevelSoundEngine::SetGameObjectAuxSendValues(
	AkGameObjectID		in_gameObjectID,
	AkAuxSendValue* in_aAuxSendValues,
	AkUInt32			in_uNumSendValues
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetGameObjectAuxSendValues(in_gameObjectID, in_aAuxSendValues, in_uNumSendValues);
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterBusVolumeCallback(
	AkUniqueID in_busID,
	AkBusCallbackFunc in_pfnCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterBusVolumeCallback(in_busID, in_pfnCallback, in_pCookie);
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterBusMeteringCallback(
	AkUniqueID in_busID,
	AkBusMeteringCallbackFunc in_pfnCallback,
	AkMeteringFlags in_eMeteringFlags,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterBusMeteringCallback(in_busID, in_pfnCallback, in_eMeteringFlags, in_pCookie);
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterOutputDeviceMeteringCallback(
	AkOutputDeviceID in_idOutput,
	AkOutputDeviceMeteringCallbackFunc in_pfnCallback,
	AkMeteringFlags in_eMeteringFlags,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterOutputDeviceMeteringCallback(in_idOutput, in_pfnCallback, in_eMeteringFlags, in_pCookie);
}

AKRESULT FWwiseLowLevelSoundEngine::SetGameObjectOutputBusVolume(
	AkGameObjectID		in_emitterObjID,
	AkGameObjectID		in_listenerObjID,
	AkReal32			in_fControlValue
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetGameObjectOutputBusVolume(in_emitterObjID, in_listenerObjID, in_fControlValue);
}

AKRESULT FWwiseLowLevelSoundEngine::SetActorMixerEffect(
	AkUniqueID in_audioNodeID,
	AkUInt32 in_uFXIndex,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetActorMixerEffect(in_audioNodeID, in_uFXIndex, in_shareSetID);
}

AKRESULT FWwiseLowLevelSoundEngine::SetBusEffect(
	AkUniqueID in_audioNodeID,
	AkUInt32 in_uFXIndex,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetBusEffect(in_audioNodeID, in_uFXIndex, in_shareSetID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::SetBusEffect(
	const wchar_t* in_pszBusName,
	AkUInt32 in_uFXIndex,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetBusEffect(in_pszBusName, in_uFXIndex, in_shareSetID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::SetBusEffect(
	const char* in_pszBusName,
	AkUInt32 in_uFXIndex,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetBusEffect(in_pszBusName, in_uFXIndex, in_shareSetID);
}

AKRESULT FWwiseLowLevelSoundEngine::SetOutputDeviceEffect(
	AkOutputDeviceID in_outputDeviceID,
	AkUInt32 in_uFXIndex,
	AkUniqueID in_FXShareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetOutputDeviceEffect(in_outputDeviceID, in_uFXIndex, in_FXShareSetID);
}

AKRESULT FWwiseLowLevelSoundEngine::SetMixer(
	AkUniqueID in_audioNodeID,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetMixer(in_audioNodeID, in_shareSetID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::SetMixer(
	const wchar_t* in_pszBusName,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetMixer(in_pszBusName, in_shareSetID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::SetMixer(
	const char* in_pszBusName,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetMixer(in_pszBusName, in_shareSetID);
}

AKRESULT FWwiseLowLevelSoundEngine::SetBusConfig(
	AkUniqueID in_audioNodeID,
	AkChannelConfig in_channelConfig
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetBusConfig(in_audioNodeID, in_channelConfig);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::SetBusConfig(
	const wchar_t* in_pszBusName,
	AkChannelConfig in_channelConfig
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetBusConfig(in_pszBusName, in_channelConfig);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::SetBusConfig(
	const char* in_pszBusName,
	AkChannelConfig in_channelConfig
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetBusConfig(in_pszBusName, in_channelConfig);
}

AKRESULT FWwiseLowLevelSoundEngine::SetObjectObstructionAndOcclusion(
	AkGameObjectID in_EmitterID,
	AkGameObjectID in_ListenerID,
	AkReal32 in_fObstructionLevel,
	AkReal32 in_fOcclusionLevel
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetObjectObstructionAndOcclusion(in_EmitterID, in_ListenerID, in_fObstructionLevel, in_fOcclusionLevel);
}

AKRESULT FWwiseLowLevelSoundEngine::SetMultipleObstructionAndOcclusion(
	AkGameObjectID in_EmitterID,
	AkGameObjectID in_uListenerID,
	AkObstructionOcclusionValues* in_fObstructionOcclusionValues,
	AkUInt32 in_uNumOcclusionObstruction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetMultipleObstructionAndOcclusion(in_EmitterID, in_uListenerID, in_fObstructionOcclusionValues, in_uNumOcclusionObstruction);
}

AKRESULT FWwiseLowLevelSoundEngine::GetContainerHistory(
	AK::IWriteBytes* in_pBytes
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetContainerHistory(in_pBytes);
}

AKRESULT FWwiseLowLevelSoundEngine::SetContainerHistory(
	AK::IReadBytes* in_pBytes
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetContainerHistory(in_pBytes);
}

AKRESULT FWwiseLowLevelSoundEngine::StartOutputCapture(
	const AkOSChar* in_CaptureFileName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::StartOutputCapture(in_CaptureFileName);
}

AKRESULT FWwiseLowLevelSoundEngine::StopOutputCapture()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::StopOutputCapture();
}

AKRESULT FWwiseLowLevelSoundEngine::AddOutputCaptureMarker(
	const char* in_MarkerText
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::AddOutputCaptureMarker(in_MarkerText);
}

AkUInt32 FWwiseLowLevelSoundEngine::GetSampleRate()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetSampleRate();
}

AKRESULT FWwiseLowLevelSoundEngine::RegisterCaptureCallback(
	AkCaptureCallbackFunc in_pfnCallback,
	AkOutputDeviceID in_idOutput,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RegisterCaptureCallback(in_pfnCallback, in_idOutput, in_pCookie);
}

AKRESULT FWwiseLowLevelSoundEngine::UnregisterCaptureCallback(
	AkCaptureCallbackFunc in_pfnCallback,
	AkOutputDeviceID in_idOutput,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::UnregisterCaptureCallback(in_pfnCallback, in_idOutput, in_pCookie);
}

AKRESULT FWwiseLowLevelSoundEngine::StartProfilerCapture(
	const AkOSChar* in_CaptureFileName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::StartProfilerCapture(in_CaptureFileName);
}

AKRESULT FWwiseLowLevelSoundEngine::StopProfilerCapture()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::StopProfilerCapture();
}

AKRESULT FWwiseLowLevelSoundEngine::SetOfflineRenderingFrameTime(
	AkReal32 in_fFrameTimeInSeconds
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetOfflineRenderingFrameTime(in_fFrameTimeInSeconds);
}

AKRESULT FWwiseLowLevelSoundEngine::SetOfflineRendering(
	bool in_bEnableOfflineRendering
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetOfflineRendering(in_bEnableOfflineRendering);
}

AKRESULT FWwiseLowLevelSoundEngine::AddOutput(
	const AkOutputSettings& in_Settings,
	AkOutputDeviceID* out_pDeviceID,
	const AkGameObjectID* in_pListenerIDs,
	AkUInt32 in_uNumListeners
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::AddOutput(in_Settings, out_pDeviceID, in_pListenerIDs, in_uNumListeners);
}

AKRESULT FWwiseLowLevelSoundEngine::RemoveOutput(
	AkOutputDeviceID in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::RemoveOutput(in_idOutput);
}

AKRESULT FWwiseLowLevelSoundEngine::ReplaceOutput(
	const AkOutputSettings& in_Settings,
	AkOutputDeviceID in_outputDeviceId,
	AkOutputDeviceID* out_pOutputDeviceId
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::ReplaceOutput(in_Settings, in_outputDeviceId, out_pOutputDeviceId);
}

AkOutputDeviceID FWwiseLowLevelSoundEngine::GetOutputID(
	AkUniqueID in_idShareset,
	AkUInt32 in_idDevice
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetOutputID(in_idShareset, in_idDevice);
}

AkOutputDeviceID FWwiseLowLevelSoundEngine::GetOutputID(
	const char* in_szShareSet,
	AkUInt32 in_idDevice
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetOutputID(in_szShareSet, in_idDevice);
}

#ifdef AK_SUPPORT_WCHAR
AkOutputDeviceID FWwiseLowLevelSoundEngine::GetOutputID(
	const wchar_t* in_szShareSet,
	AkUInt32 in_idDevice
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetOutputID(in_szShareSet, in_idDevice);
}
#endif

AKRESULT FWwiseLowLevelSoundEngine::SetBusDevice(
	AkUniqueID in_idBus,
	AkUniqueID in_idNewDevice
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetBusDevice(in_idBus, in_idNewDevice);
}

AKRESULT FWwiseLowLevelSoundEngine::SetBusDevice(
	const char* in_BusName,
	const char* in_DeviceName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetBusDevice(in_BusName, in_DeviceName);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::SetBusDevice(
	const wchar_t* in_BusName,
	const wchar_t* in_DeviceName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetBusDevice(in_BusName, in_DeviceName);
}
#endif

AKRESULT FWwiseLowLevelSoundEngine::GetDeviceList(
	AkUInt32 in_ulCompanyID,
	AkUInt32 in_ulPluginID,
	AkUInt32& io_maxNumDevices,
	AkDeviceDescription* out_deviceDescriptions
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetDeviceList(in_ulCompanyID, in_ulPluginID, io_maxNumDevices, out_deviceDescriptions);
}

AKRESULT FWwiseLowLevelSoundEngine::GetDeviceList(
	AkUniqueID in_audioDeviceShareSetID,
	AkUInt32& io_maxNumDevices,
	AkDeviceDescription* out_deviceDescriptions
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetDeviceList(in_audioDeviceShareSetID, io_maxNumDevices, out_deviceDescriptions);
}

AKRESULT FWwiseLowLevelSoundEngine::SetOutputVolume(
	AkOutputDeviceID in_idOutput,
	AkReal32 in_fVolume
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::SetOutputVolume(in_idOutput, in_fVolume);
}

AKRESULT FWwiseLowLevelSoundEngine::GetDeviceSpatialAudioSupport(
	AkUInt32 in_idDevice)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetDeviceSpatialAudioSupport(in_idDevice);
}

AKRESULT FWwiseLowLevelSoundEngine::Suspend(
	bool in_bRenderAnyway,
	bool in_bFadeOut
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Suspend(in_bRenderAnyway, in_bFadeOut);
}

AKRESULT FWwiseLowLevelSoundEngine::WakeupFromSuspend(
	AkUInt32 in_uDelayMs
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::WakeupFromSuspend(in_uDelayMs);
}

AkUInt32 FWwiseLowLevelSoundEngine::GetBufferTick()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetBufferTick();
}

AkUInt64 FWwiseLowLevelSoundEngine::GetSampleTick()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::GetSampleTick();
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetPosition(
	AkGameObjectID in_GameObjectID,
	AkSoundPosition& out_rPosition
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetPosition(in_GameObjectID, out_rPosition);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetListeners(
	AkGameObjectID in_GameObjectID,
	AkGameObjectID* out_ListenerObjectIDs,
	AkUInt32& oi_uNumListeners
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetListeners(in_GameObjectID, out_ListenerObjectIDs, oi_uNumListeners);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetListenerPosition(
	AkGameObjectID in_uIndex,
	AkListenerPosition& out_rPosition
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetListenerPosition(in_uIndex, out_rPosition);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetListenerSpatialization(
	AkUInt32 in_uIndex,
	bool& out_rbSpatialized,
	AK::SpeakerVolumes::VectorPtr& out_pVolumeOffsets,
	AkChannelConfig& out_channelConfig
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetListenerSpatialization(in_uIndex, out_rbSpatialized, out_pVolumeOffsets, out_channelConfig);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetRTPCValue(
	AkRtpcID in_rtpcID,
	AkGameObjectID in_gameObjectID,
	AkPlayingID	in_playingID,
	AkRtpcValue& out_rValue,
	AK::SoundEngine::Query::RTPCValue_type& io_rValueType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetRTPCValue(in_rtpcID, in_gameObjectID, in_playingID, out_rValue, io_rValueType);
}

#ifdef AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetRTPCValue(
	const wchar_t* in_pszRtpcName,
	AkGameObjectID in_gameObjectID,
	AkPlayingID	in_playingID,
	AkRtpcValue& out_rValue,
	AK::SoundEngine::Query::RTPCValue_type& io_rValueType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetRTPCValue(in_pszRtpcName, in_gameObjectID, in_playingID, out_rValue, io_rValueType);
}

#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetRTPCValue(
	const char* in_pszRtpcName,
	AkGameObjectID in_gameObjectID,
	AkPlayingID	in_playingID,
	AkRtpcValue& out_rValue,
	AK::SoundEngine::Query::RTPCValue_type& io_rValueType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetRTPCValue(in_pszRtpcName, in_gameObjectID, in_playingID, out_rValue, io_rValueType);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetSwitch(
	AkSwitchGroupID in_switchGroup,
	AkGameObjectID  in_gameObjectID,
	AkSwitchStateID& out_rSwitchState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetSwitch(in_switchGroup, in_gameObjectID, out_rSwitchState);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetSwitch(
	const wchar_t* in_pstrSwitchGroupName,
	AkGameObjectID in_GameObj,
	AkSwitchStateID& out_rSwitchState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetSwitch(in_pstrSwitchGroupName, in_GameObj, out_rSwitchState);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetSwitch(
	const char* in_pstrSwitchGroupName,
	AkGameObjectID in_GameObj,
	AkSwitchStateID& out_rSwitchState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetSwitch(in_pstrSwitchGroupName, in_GameObj, out_rSwitchState);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetState(
	AkStateGroupID in_stateGroup,
	AkStateID& out_rState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetState(in_stateGroup, out_rState);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetState(
	const wchar_t* in_pstrStateGroupName,
	AkStateID& out_rState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetState(in_pstrStateGroupName, out_rState);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetState(
	const char* in_pstrStateGroupName,
	AkStateID& out_rState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetState(in_pstrStateGroupName, out_rState);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetGameObjectAuxSendValues(
	AkGameObjectID		in_gameObjectID,
	AkAuxSendValue* out_paAuxSendValues,
	AkUInt32& io_ruNumSendValues
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetGameObjectAuxSendValues(in_gameObjectID, out_paAuxSendValues, io_ruNumSendValues);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetGameObjectDryLevelValue(
	AkGameObjectID		in_EmitterID,
	AkGameObjectID		in_ListenerID,
	AkReal32& out_rfControlValue
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetGameObjectDryLevelValue(in_EmitterID, in_ListenerID, out_rfControlValue);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetObjectObstructionAndOcclusion(
	AkGameObjectID in_EmitterID,
	AkGameObjectID in_ListenerID,
	AkReal32& out_rfObstructionLevel,
	AkReal32& out_rfOcclusionLevel
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetObjectObstructionAndOcclusion(in_EmitterID, in_ListenerID, out_rfObstructionLevel, out_rfOcclusionLevel);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::QueryAudioObjectIDs(
	AkUniqueID in_eventID,
	AkUInt32& io_ruNumItems,
	AkObjectInfo* out_aObjectInfos
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::QueryAudioObjectIDs(in_eventID, io_ruNumItems, out_aObjectInfos);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseLowLevelSoundEngine::FQuery::QueryAudioObjectIDs(
	const wchar_t* in_pszEventName,
	AkUInt32& io_ruNumItems,
	AkObjectInfo* out_aObjectInfos
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::QueryAudioObjectIDs(in_pszEventName, io_ruNumItems, out_aObjectInfos);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseLowLevelSoundEngine::FQuery::QueryAudioObjectIDs(
	const char* in_pszEventName,
	AkUInt32& io_ruNumItems,
	AkObjectInfo* out_aObjectInfos
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::QueryAudioObjectIDs(in_pszEventName, io_ruNumItems, out_aObjectInfos);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetPositioningInfo(
	AkUniqueID in_ObjectID,
	AkPositioningInfo& out_rPositioningInfo
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetPositioningInfo(in_ObjectID, out_rPositioningInfo);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetActiveGameObjects(
	AK::SoundEngine::Query::AkGameObjectsList& io_GameObjectList
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetActiveGameObjects(io_GameObjectList);
}

bool FWwiseLowLevelSoundEngine::FQuery::GetIsGameObjectActive(
	AkGameObjectID in_GameObjId
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetIsGameObjectActive(in_GameObjId);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetMaxRadius(
	AK::SoundEngine::Query::AkRadiusList& io_RadiusList
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetMaxRadius(io_RadiusList);
}

AkReal32 FWwiseLowLevelSoundEngine::FQuery::GetMaxRadius(
	AkGameObjectID in_GameObjId
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetMaxRadius(in_GameObjId);
}

AkUniqueID FWwiseLowLevelSoundEngine::FQuery::GetEventIDFromPlayingID(
	AkPlayingID in_playingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetEventIDFromPlayingID(in_playingID);
}

AkGameObjectID FWwiseLowLevelSoundEngine::FQuery::GetGameObjectFromPlayingID(
	AkPlayingID in_playingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetGameObjectFromPlayingID(in_playingID);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetPlayingIDsFromGameObject(
	AkGameObjectID in_GameObjId,
	AkUInt32& io_ruNumIDs,
	AkPlayingID* out_aPlayingIDs
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetPlayingIDsFromGameObject(in_GameObjId, io_ruNumIDs, out_aPlayingIDs);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetCustomPropertyValue(
	AkUniqueID in_ObjectID,
	AkUInt32 in_uPropID,
	AkInt32& out_iValue
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetCustomPropertyValue(in_ObjectID, in_uPropID, out_iValue);
}

AKRESULT FWwiseLowLevelSoundEngine::FQuery::GetCustomPropertyValue(
	AkUniqueID in_ObjectID,
	AkUInt32 in_uPropID,
	AkReal32& out_fValue
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SoundEngine::Query::GetCustomPropertyValue(in_ObjectID, in_uPropID, out_fValue);
}

void FWwiseLowLevelSoundEngine::FAkAudioInputPlugin::SetAudioInputCallbacks(
	AkAudioInputPluginExecuteCallbackFunc in_pfnExecCallback,
	AkAudioInputPluginGetFormatCallbackFunc in_pfnGetFormatCallback /*= nullptr */,
	AkAudioInputPluginGetGainCallbackFunc in_pfnGetGainCallback /*= nullptr*/
)
{
	::SetAudioInputCallbacks(in_pfnExecCallback, in_pfnGetFormatCallback, in_pfnGetGainCallback);
}