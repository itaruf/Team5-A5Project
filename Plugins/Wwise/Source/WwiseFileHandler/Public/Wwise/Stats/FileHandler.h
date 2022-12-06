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

#pragma once

#include "Stats/Stats.h"
#include "Logging/LogMacros.h"

DECLARE_STATS_GROUP(TEXT("File Handler"), STATGROUP_WwiseFileHandler, STATCAT_Wwise);

DECLARE_MEMORY_STAT_EXTERN(TEXT("Memory Allocated"), STAT_WwiseFileHandlerMemoryAllocated, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Memory Mapped"), STAT_WwiseFileHandlerMemoryMapped, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Device Memory Allocated"), STAT_WwiseFileHandlerDeviceMemoryAllocated, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Created External Source States"), STAT_WwiseFileHandlerCreatedExternalSourceStates, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Known External Source Media"), STAT_WwiseFileHandlerKnownExternalSourceMedia, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Loaded External Source Media"), STAT_WwiseFileHandlerLoadedExternalSourceMedia, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Known Media"), STAT_WwiseFileHandlerKnownMedia, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Loaded Media"), STAT_WwiseFileHandlerLoadedMedia, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Known SoundBanks"), STAT_WwiseFileHandlerKnownSoundBanks, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Loaded SoundBanks"), STAT_WwiseFileHandlerLoadedSoundBanks, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Total Error Count"), STAT_WwiseFileHandlerTotalErrorCount, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("State Operations Being Processed"), STAT_WwiseFileHandlerStateOperationsBeingProcessed, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("State Operation Latency"), STAT_WwiseFileHandlerStateOperationLatency, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);

DECLARE_STATS_GROUP(TEXT("File Handler - Low-level I/O"), STATGROUP_WwiseFileHandlerLowLevelIO, STATCAT_Wwise);

DECLARE_FLOAT_COUNTER_STAT_EXTERN(TEXT("Streaming KB / Frame"), STAT_WwiseFileHandlerStreamingKB, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_FLOAT_COUNTER_STAT_EXTERN(TEXT("Preloading KB / Frame"), STAT_WwiseFileHandlerPreloadingKB, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Opened Streams"), STAT_WwiseFileHandlerOpenedStreams, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Pending Requests"), STAT_WwiseFileHandlerPendingRequests, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Total Requests"), STAT_WwiseFileHandlerTotalRequests, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_FLOAT_ACCUMULATOR_STAT_EXTERN(TEXT("Total Streaming MB"), STAT_WwiseFileHandlerTotalStreamedMB, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("IO Request Latency"), STAT_WwiseFileHandlerIORequestLatency, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("File Operation Latency"), STAT_WwiseFileHandlerFileOperationLatency, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("SoundEngine Callback Latency"), STAT_WwiseFileHandlerSoundEngineCallbackLatency, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);

WWISEFILEHANDLER_API DECLARE_LOG_CATEGORY_EXTERN(LogWwiseFileHandler, Log, All);
