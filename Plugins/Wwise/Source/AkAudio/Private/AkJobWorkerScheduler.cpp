/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#include "AkJobWorkerScheduler.h"
#include "AkAudioDevice.h"

#define AK_DECLARE_JOB_TYPE(__job__, __desc__, __thread__) \
	DECLARE_CYCLE_STAT(TEXT(__desc__), STAT_AkJob##__job__, STATGROUP_Audio); \
	static ENamedThreads::Type kThread##__job__ = __thread__;

#define AK_DEFINE_JOB_CASE(__job__) \
	case AkJobType_##__job__: \
		for (int i=0; i < (int)in_uNumWorkers; i++) { \
			FFunctionGraphTask::CreateAndDispatchWhenReady([=]() { \
				in_fnJobWorker(AkJobType_##__job__, uMaxExecutionTime); \
			}, GET_STATID(STAT_AkJob##__job__), nullptr, kThread##__job__); \
		} \
		break;

static_assert(AK_NUM_JOB_TYPES == 3, "Update the stat groups and switch cases below for new job types!");
AK_DECLARE_JOB_TYPE(Generic, "Wwise Generic Job", ENamedThreads::AnyHiPriThreadNormalTask);
AK_DECLARE_JOB_TYPE(AudioProcessing, "Wwise Audio Processing Job", ENamedThreads::AnyHiPriThreadNormalTask);
AK_DECLARE_JOB_TYPE(SpatialAudio, "Wwise Spatial Audio Job", ENamedThreads::AnyHiPriThreadNormalTask);

static void OnJobWorkerRequest(AkJobWorkerFunc in_fnJobWorker, AkJobType in_jobType, AkUInt32 in_uNumWorkers, void* in_pUserData)
{
	FAkJobWorkerScheduler* pScheduler = static_cast<FAkJobWorkerScheduler*>(in_pUserData);
	AkUInt32 uMaxExecutionTime = pScheduler->uMaxExecutionTime;
	switch (in_jobType)
	{
		AK_DEFINE_JOB_CASE(Generic);
		AK_DEFINE_JOB_CASE(AudioProcessing);
		AK_DEFINE_JOB_CASE(SpatialAudio);
	}
}

void FAkJobWorkerScheduler::InstallJobWorkerScheduler(uint32 in_uMaxExecutionTime, AkJobMgrSettings & out_settings)
{
	if (!FTaskGraphInterface::Get().IsRunning())
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Multi-core rendering was requested, but Task Graph is not running. Multi-core rendering disabled."));
	}
	else if (!FPlatformProcess::SupportsMultithreading())
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Multi-core rendering was requested, platform does not support multi-threading. Multi-core rendering disabled."));
	}
	else
	{
		check(ENamedThreads::bHasHighPriorityThreads);
		uMaxExecutionTime = in_uMaxExecutionTime;

		out_settings.fnRequestJobWorker = OnJobWorkerRequest;
		out_settings.pClientData = this;
		AkUInt32 uMaxActiveWorkers = FTaskGraphInterface::Get().GetNumWorkerThreads();
		for (int i = 0; i < AK_NUM_JOB_TYPES; i++)
		{
			out_settings.uMaxActiveWorkers[i] = uMaxActiveWorkers;
		}
	}
}
