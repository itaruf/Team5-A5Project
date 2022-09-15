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


/*=============================================================================
	AkIncludes.h:
=============================================================================*/

/*------------------------------------------------------------------------------------
	Audio includes.
------------------------------------------------------------------------------------*/

#pragma once
#include "Engine/Engine.h"
#include "AkUEFeatures.h"

#if PLATFORM_WINDOWS

// Currently, Wwise SDK builds with the default 8 byte alignment, whereas Unreal builds with 4 byte alignment under VC.
// This causes data corruption if the headers are not included with forced alignment directives.
// http://msdn.microsoft.com/en-us/library/xh3e3fd0%28VS.80%29.aspx
#pragma pack(push, 8)
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include "Windows/MinimalWindowsApi.h"
#elif PLATFORM_HOLOLENS
#pragma pack(push, 8)
#include "Hololens/AllowWindowsPlatformTypes.h"
#include "Hololens/AllowWindowsPlatformAtomics.h"
#elif defined(AK_XBOXONE_COMMON)
#include "Microsoft/AllowMicrosoftPlatformTypes.h"
#include "Microsoft/AllowMicrosoftPlatformAtomics.h"
#elif defined(AK_NEED_XBOX_MINAPI) && AK_NEED_XBOX_MINAPI
#include "XboxOne/XboxOneMinApi.h"
#endif


#include "CoreTypes.h" // includes platform defines

#include <AK/AkWwiseSDKVersion.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>
#include <AK/IBytes.h>
#include <AK/SoundEngine/Common/AkVirtualAcoustics.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/AkNumeralTypes.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/Tools/Common/AkArray.h>
#include <AK/Tools/Common/AkInstrument.h>
#include <AK/Tools/Common/AkMonitorError.h>
#include <AK/Tools/Common/AkObject.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <AK/Comm/AkCommunication.h>
#include <AK/Plugin/AkAudioInputPlugin.h>

#include <AK/SoundEngine/Common/AkQueryParameters.h>

#if WITH_EDITOR
#if AK_SUPPORT_WAAPI
#include <AK/WwiseAuthoringAPI/AkAutobahn/AkWAAPIErrorMessageTranslator.h>
#endif
#endif

#if AK_SUPPORT_WAAPI
#include <AK/WwiseAuthoringAPI/AkAutobahn/Client.h>
#include <AK/WwiseAuthoringAPI/waapi.h>
#endif

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformAtomics.h"
#pragma pack(pop)
#elif PLATFORM_HOLOLENS
#include "Hololens/HideWindowsPlatformTypes.h"
#include "Hololens/HideWindowsPlatformAtomics.h"
#pragma pack(pop)
#elif defined(AK_XBOXONE_COMMON)
#include "Microsoft/HideMicrosoftPlatformTypes.h"
#include "Microsoft/HideMicrosoftPlatformAtomics.h"
#endif // PLATFORM_WINDOWS
