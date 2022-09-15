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

#include "Stats/Stats.h"
#include "Logging/LogMacros.h"

DECLARE_STATS_GROUP(TEXT("SoundEngine"), STATGROUP_WwiseSoundEngine, STATCAT_Wwise);
DECLARE_CYCLE_STAT_EXTERN(TEXT("SoundEngine API Calls"), STAT_WwiseLowLevelSoundEngine, STATGROUP_WwiseSoundEngine, WWISESOUNDENGINE_API);

WWISESOUNDENGINE_API DECLARE_LOG_CATEGORY_EXTERN(LogWwiseSoundEngine, Log, All);
