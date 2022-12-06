/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

#include "Wwise/API_2022_1/WwiseCommAPI_2022_1.h"
#include "Wwise/Stats/SoundEngine_2022_1.h"

AKRESULT FWwiseCommAPI_2022_1::Init(
	const AkCommSettings& in_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#ifdef AK_OPTIMIZED
	return AK_NotImplemented;
#else
	return AK::Comm::Init(in_settings);
#endif
}

AkInt32 FWwiseCommAPI_2022_1::GetLastError()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#ifdef AK_OPTIMIZED
	return 0;
#else
	return 0;
	//return AK::Comm::GetLastError();
#endif
}

void FWwiseCommAPI_2022_1::GetDefaultInitSettings(
	AkCommSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#ifdef AK_OPTIMIZED
	return;
#else
	AK::Comm::GetDefaultInitSettings(out_settings);
#endif
}

void FWwiseCommAPI_2022_1::Term()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#ifdef AK_OPTIMIZED
	return;
#else
	AK::Comm::Term();
#endif
}

AKRESULT FWwiseCommAPI_2022_1::Reset()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#ifdef AK_OPTIMIZED
	return AK_NotImplemented;
#else
	return AK::Comm::Reset();
#endif
}

const AkCommSettings& FWwiseCommAPI_2022_1::GetCurrentSettings()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#ifdef AK_OPTIMIZED
	static const AkCommSettings StaticSettings;
	return StaticSettings;
#else
	return AK::Comm::GetCurrentSettings();
#endif
}
