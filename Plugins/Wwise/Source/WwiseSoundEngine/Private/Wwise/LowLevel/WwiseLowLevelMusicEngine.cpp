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


#include "Wwise/LowLevel/WwiseLowLevelMusicEngine.h"
#include "Wwise/Stats/SoundEngine.h"

AKRESULT FWwiseLowLevelMusicEngine::Init(
	AkMusicSettings* in_pSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::MusicEngine::Init(in_pSettings);
}

void FWwiseLowLevelMusicEngine::GetDefaultInitSettings(
	AkMusicSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MusicEngine::GetDefaultInitSettings(out_settings);
}

void FWwiseLowLevelMusicEngine::Term(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MusicEngine::Term();
}

AKRESULT FWwiseLowLevelMusicEngine::GetPlayingSegmentInfo(
	AkPlayingID		in_PlayingID,
	AkSegmentInfo& out_segmentInfo,
	bool			in_bExtrapolate
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::MusicEngine::GetPlayingSegmentInfo(in_PlayingID, out_segmentInfo, in_bExtrapolate);
}
