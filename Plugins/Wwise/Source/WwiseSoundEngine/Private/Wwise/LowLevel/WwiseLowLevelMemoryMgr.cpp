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


#include "Wwise/LowLevel/WwiseLowLevelMemoryMgr.h"
#include "Wwise/Stats/SoundEngine.h"

AKRESULT FWwiseLowLevelMemoryMgr::Init(
	AkMemSettings* in_pSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::MemoryMgr::Init(in_pSettings);
}

void FWwiseLowLevelMemoryMgr::GetDefaultSettings(
	AkMemSettings& out_pMemSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MemoryMgr::GetDefaultSettings(out_pMemSettings);
}

bool FWwiseLowLevelMemoryMgr::IsInitialized()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::MemoryMgr::IsInitialized();
}

void FWwiseLowLevelMemoryMgr::Term()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MemoryMgr::Term();
}

void FWwiseLowLevelMemoryMgr::InitForThread()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MemoryMgr::InitForThread();
}

void FWwiseLowLevelMemoryMgr::TermForThread()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MemoryMgr::TermForThread();
}

void* FWwiseLowLevelMemoryMgr::dMalloc(
	AkMemPoolId in_poolId,
	size_t		in_uSize,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
#ifdef AK_MEMDEBUG
	return AK::MemoryMgr::dMalloc(in_poolId, in_uSize, in_pszFile, in_uLine);
#else
	return AK::MemoryMgr::Malloc(in_poolId, in_uSize);
#endif
}

void* FWwiseLowLevelMemoryMgr::Malloc(
	AkMemPoolId in_poolId,
	size_t		in_uSize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::MemoryMgr::Malloc(in_poolId, in_uSize);
}

void* FWwiseLowLevelMemoryMgr::dRealloc(
	AkMemPoolId	in_poolId,
	void* in_pAlloc,
	size_t		in_uSize,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
#ifdef AK_MEMDEBUG
	return AK::MemoryMgr::dRealloc(in_poolId, in_pAlloc, in_uSize, in_pszFile, in_uLine);
#else
	return AK::MemoryMgr::Realloc(in_poolId, in_pAlloc, in_uSize);
#endif
}

void* FWwiseLowLevelMemoryMgr::Realloc(
	AkMemPoolId in_poolId,
	void* in_pAlloc,
	size_t		in_uSize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::MemoryMgr::Realloc(in_poolId, in_pAlloc, in_uSize);
}

void* FWwiseLowLevelMemoryMgr::dReallocAligned(
	AkMemPoolId	in_poolId,
	void* in_pAlloc,
	size_t		in_uSize,
	AkUInt32	in_uAlignment,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
#ifdef AK_MEMDEBUG
	return AK::MemoryMgr::dReallocAligned(in_poolId, in_pAlloc, in_uSize, in_uAlignment, in_pszFile, in_uLine);
#else
	return AK::MemoryMgr::ReallocAligned(in_poolId, in_pAlloc, in_uSize, in_uAlignment);
#endif
}

void* FWwiseLowLevelMemoryMgr::ReallocAligned(
	AkMemPoolId in_poolId,
	void* in_pAlloc,
	size_t		in_uSize,
	AkUInt32	in_uAlignment
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::MemoryMgr::ReallocAligned(in_poolId, in_pAlloc, in_uSize, in_uAlignment);
}

void FWwiseLowLevelMemoryMgr::Free(
	AkMemPoolId in_poolId,
	void* in_pMemAddress
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MemoryMgr::Free(in_poolId, in_pMemAddress);
}

void* FWwiseLowLevelMemoryMgr::dMalign(
	AkMemPoolId in_poolId,
	size_t		in_uSize,
	AkUInt32	in_uAlignment,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
#ifdef AK_MEMDEBUG
	return AK::MemoryMgr::dMalign(in_poolId, in_uSize, in_uAlignment, in_pszFile, in_uLine);
#else
	return AK::MemoryMgr::Malign(in_poolId, in_uSize, in_uAlignment);
#endif
}

void* FWwiseLowLevelMemoryMgr::Malign(
	AkMemPoolId in_poolId,
	size_t		in_uSize,
	AkUInt32	in_uAlignment
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::MemoryMgr::Malign(in_poolId, in_uSize, in_uAlignment);
}

void FWwiseLowLevelMemoryMgr::GetCategoryStats(
	AkMemPoolId	in_poolId,
	AK::MemoryMgr::CategoryStats& out_poolStats
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MemoryMgr::GetCategoryStats(in_poolId, out_poolStats);
}

void FWwiseLowLevelMemoryMgr::GetGlobalStats(
	AK::MemoryMgr::GlobalStats& out_stats
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MemoryMgr::GetGlobalStats(out_stats);
}

void FWwiseLowLevelMemoryMgr::StartProfileThreadUsage(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MemoryMgr::StartProfileThreadUsage();
}

AkUInt64 StopProfileThreadUsage(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::MemoryMgr::StopProfileThreadUsage();
}

void FWwiseLowLevelMemoryMgr::DumpToFile(
	const AkOSChar* pszFilename
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::MemoryMgr::DumpToFile(pszFilename);
}

