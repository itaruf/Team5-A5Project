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

DECLARE_STATS_GROUP(TEXT("WwiseProjectDatabase"), STATGROUP_WwiseProjectDatabase, STATCAT_Wwise);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Memory"), STAT_WwiseProjectDatabaseMemory, STATGROUP_WwiseProjectDatabase, WWISEPROJECTDATABASE_API);

WWISEPROJECTDATABASE_API DECLARE_LOG_CATEGORY_EXTERN(LogWwiseProjectDatabase, Log, All);
