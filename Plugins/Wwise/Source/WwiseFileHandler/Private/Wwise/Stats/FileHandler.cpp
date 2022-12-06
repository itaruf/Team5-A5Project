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

#include "Wwise/Stats/FileHandler.h"

DEFINE_STAT(STAT_WwiseFileHandlerMemoryAllocated);
DEFINE_STAT(STAT_WwiseFileHandlerMemoryMapped);

#if AK_SUPPORT_DEVICE_MEMORY
DEFINE_STAT(STAT_WwiseFileHandlerDeviceMemoryAllocated);
#endif

DEFINE_STAT(STAT_WwiseFileHandlerCreatedExternalSourceStates);
DEFINE_STAT(STAT_WwiseFileHandlerKnownExternalSourceMedia);
DEFINE_STAT(STAT_WwiseFileHandlerLoadedExternalSourceMedia);
DEFINE_STAT(STAT_WwiseFileHandlerKnownMedia);
DEFINE_STAT(STAT_WwiseFileHandlerLoadedMedia);
DEFINE_STAT(STAT_WwiseFileHandlerKnownSoundBanks);
DEFINE_STAT(STAT_WwiseFileHandlerLoadedSoundBanks);

DEFINE_STAT(STAT_WwiseFileHandlerTotalErrorCount);
DEFINE_STAT(STAT_WwiseFileHandlerStateOperationsBeingProcessed);
DEFINE_STAT(STAT_WwiseFileHandlerStateOperationLatency);

DEFINE_STAT(STAT_WwiseFileHandlerStreamingKB);
DEFINE_STAT(STAT_WwiseFileHandlerPreloadingKB);
DEFINE_STAT(STAT_WwiseFileHandlerOpenedStreams);
DEFINE_STAT(STAT_WwiseFileHandlerPendingRequests);
DEFINE_STAT(STAT_WwiseFileHandlerTotalRequests);
DEFINE_STAT(STAT_WwiseFileHandlerTotalStreamedMB);

DEFINE_STAT(STAT_WwiseFileHandlerIORequestLatency); 
DEFINE_STAT(STAT_WwiseFileHandlerFileOperationLatency);
DEFINE_STAT(STAT_WwiseFileHandlerSoundEngineCallbackLatency);


DEFINE_LOG_CATEGORY(LogWwiseFileHandler);
