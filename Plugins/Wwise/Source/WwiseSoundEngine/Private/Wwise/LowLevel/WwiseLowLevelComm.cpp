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


#include "Wwise/LowLevel/WwiseLowLevelComm.h"
#include "Wwise/Stats/SoundEngine.h"

AKRESULT FWwiseLowLevelComm::Init(
	const AkCommSettings& in_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
#ifdef AK_OPTIMIZED
	return AK_NotImplemented;
#else
	return AK::Comm::Init(in_settings);
#endif
}

AkInt32 FWwiseLowLevelComm::GetLastError()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
#ifdef AK_OPTIMIZED
	return 0;
#else
	return 0;
	//return AK::Comm::GetLastError();
#endif
}

void FWwiseLowLevelComm::GetDefaultInitSettings(
	AkCommSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
#ifdef AK_OPTIMIZED
	return;
#else
	AK::Comm::GetDefaultInitSettings(out_settings);
#endif
}

void FWwiseLowLevelComm::Term()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
#ifdef AK_OPTIMIZED
	return;
#else
	AK::Comm::Term();
#endif
}

AKRESULT FWwiseLowLevelComm::Reset()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
#ifdef AK_OPTIMIZED
	return AK_NotImplemented;
#else
	return AK::Comm::Reset();
#endif
}

const AkCommSettings& FWwiseLowLevelComm::GetCurrentSettings()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
#ifdef AK_OPTIMIZED
	static const AkCommSettings StaticSettings;
	return StaticSettings;
#else
	return AK::Comm::GetCurrentSettings();
#endif
}
