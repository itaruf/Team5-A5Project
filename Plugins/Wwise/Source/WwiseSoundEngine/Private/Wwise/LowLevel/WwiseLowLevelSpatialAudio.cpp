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

#include "Wwise/LowLevel/WwiseLowLevelSpatialAudio.h"
#include "Wwise/Stats/SoundEngine.h"

#include <AK/SpatialAudio/Common/AkReverbEstimation.h>

AKRESULT FWwiseLowLevelSpatialAudio::Init(const AkSpatialAudioInitSettings& in_initSettings)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::Init(in_initSettings);
}

AKRESULT FWwiseLowLevelSpatialAudio::RegisterListener(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::RegisterListener(in_gameObjectID);
}

AKRESULT FWwiseLowLevelSpatialAudio::UnregisterListener(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::UnregisterListener(in_gameObjectID);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetGameObjectRadius(
	AkGameObjectID in_gameObjectID,
	AkReal32 in_outerRadius,
	AkReal32 in_innerRadius
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetGameObjectRadius(in_gameObjectID, in_outerRadius, in_innerRadius);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetImageSource(
	AkImageSourceID in_srcID,
	const AkImageSourceSettings& in_info,
	const char* in_name,
	AkUniqueID in_AuxBusID,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetImageSource(in_srcID, in_info, in_name, in_AuxBusID, in_gameObjectID);
}

AKRESULT FWwiseLowLevelSpatialAudio::RemoveImageSource(
	AkImageSourceID in_srcID,
	AkUniqueID in_AuxBusID,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::RemoveImageSource(in_srcID, in_AuxBusID, in_gameObjectID);
}

AKRESULT FWwiseLowLevelSpatialAudio::ClearImageSources(
	AkUniqueID in_AuxBusID,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::ClearImageSources(in_AuxBusID, in_gameObjectID);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetGeometry(
	AkGeometrySetID in_GeomSetID,
	const AkGeometryParams& in_params
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetGeometry(in_GeomSetID, in_params);
}

AKRESULT FWwiseLowLevelSpatialAudio::RemoveGeometry(
	AkGeometrySetID in_SetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::RemoveGeometry(in_SetID);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetGeometryInstance(
	AkGeometryInstanceID in_GeometryInstanceID,
	const AkGeometryInstanceParams& in_params
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetGeometryInstance(in_GeometryInstanceID, in_params);
}

AKRESULT FWwiseLowLevelSpatialAudio::RemoveGeometryInstance(
	AkGeometryInstanceID in_GeometryInstanceID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::RemoveGeometryInstance(in_GeometryInstanceID);
}

AKRESULT FWwiseLowLevelSpatialAudio::QueryReflectionPaths(
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_positionIndex,
	AkVector64& out_listenerPos,
	AkVector64& out_emitterPos,
	AkReflectionPathInfo* out_aPaths,
	AkUInt32& io_uArraySize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::QueryReflectionPaths(in_gameObjectID, in_positionIndex, out_listenerPos, out_emitterPos, out_aPaths, io_uArraySize);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetRoom(
	AkRoomID in_RoomID,
	const AkRoomParams& in_Params,
	const char* in_RoomName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetRoom(in_RoomID, in_Params, in_RoomName);
}

AKRESULT FWwiseLowLevelSpatialAudio::RemoveRoom(
	AkRoomID in_RoomID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::RemoveRoom(in_RoomID);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetPortal(
	AkPortalID in_PortalID,
	const AkPortalParams& in_Params,
	const char* in_PortalName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetPortal(in_PortalID, in_Params, in_PortalName);
}

AKRESULT FWwiseLowLevelSpatialAudio::RemovePortal(
	AkPortalID in_PortalID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::RemovePortal(in_PortalID);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetGameObjectInRoom(
	AkGameObjectID in_gameObjectID,
	AkRoomID in_CurrentRoomID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetGameObjectInRoom(in_gameObjectID, in_CurrentRoomID);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetReflectionsOrder(
	AkUInt32 in_uReflectionsOrder,
	bool in_bUpdatePaths
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetReflectionsOrder(in_uReflectionsOrder, in_bUpdatePaths);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetDiffractionOrder(
	AkUInt32 in_uDiffractionOrder,
	bool in_bUpdatePaths
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetDiffractionOrder(in_uDiffractionOrder, in_bUpdatePaths);
}
AKRESULT FWwiseLowLevelSpatialAudio::SetNumberOfPrimaryRays(
	AkUInt32 in_uNbPrimaryRays
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetNumberOfPrimaryRays(in_uNbPrimaryRays);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetLoadBalancingSpread(
	AkUInt32 in_uNbFrames
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetLoadBalancingSpread(in_uNbFrames);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetEarlyReflectionsAuxSend(
	AkGameObjectID in_gameObjectID,
	AkAuxBusID in_auxBusID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetEarlyReflectionsAuxSend(in_gameObjectID, in_auxBusID);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetEarlyReflectionsVolume(
	AkGameObjectID in_gameObjectID,
	AkReal32 in_fSendVolume
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetEarlyReflectionsVolume(in_gameObjectID, in_fSendVolume);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetPortalObstructionAndOcclusion(
	AkPortalID in_PortalID,
	AkReal32 in_fObstruction,
	AkReal32 in_fOcclusion
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetPortalObstructionAndOcclusion(in_PortalID, in_fObstruction, in_fOcclusion);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetGameObjectToPortalObstruction(
	AkGameObjectID in_gameObjectID,
	AkPortalID in_PortalID,
	AkReal32 in_fObstruction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetGameObjectToPortalObstruction(in_gameObjectID, in_PortalID, in_fObstruction);
}

AKRESULT FWwiseLowLevelSpatialAudio::SetPortalToPortalObstruction(
	AkPortalID in_PortalID0,
	AkPortalID in_PortalID1,
	AkReal32 in_fObstruction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::SetPortalToPortalObstruction(in_PortalID0, in_PortalID1, in_fObstruction);
}

AKRESULT FWwiseLowLevelSpatialAudio::QueryWetDiffraction(
	AkPortalID in_portal,
	AkReal32& out_wetDiffraction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::QueryWetDiffraction(in_portal, out_wetDiffraction);
}

AKRESULT FWwiseLowLevelSpatialAudio::QueryDiffractionPaths(
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_positionIndex,
	AkVector64& out_listenerPos,
	AkVector64& out_emitterPos,
	AkDiffractionPathInfo* out_aPaths,
	AkUInt32& io_uArraySize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::QueryDiffractionPaths(in_gameObjectID, in_positionIndex, out_listenerPos, out_emitterPos, out_aPaths, io_uArraySize);
}

AKRESULT FWwiseLowLevelSpatialAudio::ResetStochasticEngine()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::ResetStochasticEngine();
}

float FWwiseLowLevelSpatialAudio::FReverbEstimation::CalculateSlope(const AkAcousticTexture& texture)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::ReverbEstimation::CalculateSlope(texture);
}

void FWwiseLowLevelSpatialAudio::FReverbEstimation::GetAverageAbsorptionValues(AkAcousticTexture* in_textures, float* in_surfaceAreas, int in_numTextures, AkAcousticTexture& out_average)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	AK::SpatialAudio::ReverbEstimation::GetAverageAbsorptionValues(in_textures, in_surfaceAreas, in_numTextures, out_average);
}

AKRESULT FWwiseLowLevelSpatialAudio::FReverbEstimation::EstimateT60Decay(
	AkReal32 in_volumeCubicMeters,
	AkReal32 in_surfaceAreaSquaredMeters,
	AkReal32 in_environmentAverageAbsorption,
	AkReal32& out_decayEstimate
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::ReverbEstimation::EstimateT60Decay(in_volumeCubicMeters, in_surfaceAreaSquaredMeters, in_environmentAverageAbsorption, out_decayEstimate);
}

AKRESULT FWwiseLowLevelSpatialAudio::FReverbEstimation::EstimateTimeToFirstReflection(
	AkVector in_environmentExtentMeters,
	AkReal32& out_timeToFirstReflectionMs,
	AkReal32 in_speedOfSound
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::ReverbEstimation::EstimateTimeToFirstReflection(in_environmentExtentMeters, out_timeToFirstReflectionMs, in_speedOfSound);
}

AKRESULT FWwiseLowLevelSpatialAudio::FReverbEstimation::EstimateHFDamping(
	AkAcousticTexture* in_textures,
	float* in_surfaceAreas,
	int in_numTextures,
	AkReal32& out_hfDamping
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseLowLevelSoundEngine);
	return AK::SpatialAudio::ReverbEstimation::EstimateHFDamping(in_textures, in_surfaceAreas, in_numTextures, out_hfDamping);
}
