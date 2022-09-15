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

DECLARE_STATS_GROUP(TEXT("Resource Loader"), STATGROUP_WwiseResourceLoader, STATCAT_Wwise);

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Aux Busses"), STAT_WwiseResourceLoaderAuxBusses, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Events"), STAT_WwiseResourceLoaderEvents, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced External Sources"), STAT_WwiseResourceLoaderExternalSources, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Group Values"), STAT_WwiseResourceLoaderGroupValues, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Init Banks"), STAT_WwiseResourceLoaderInitBanks, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Media"), STAT_WwiseResourceLoaderMedia, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Sharesets"), STAT_WwiseResourceLoaderSharesets, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced SoundBanks"), STAT_WwiseResourceLoaderSoundBanks, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Switch Container Combinations"), STAT_WwiseResourceLoaderSwitchContainerCombinations, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Resource Loading"), STAT_WwiseResourceLoaderTiming, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);

WWISERESOURCELOADER_API DECLARE_LOG_CATEGORY_EXTERN(LogWwiseResourceLoader, Log, All);
