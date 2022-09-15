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

#include "Wwise/Ref/WwiseRefGameParameter.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataGameParameter.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"

const TCHAR* const FWwiseRefGameParameter::NAME = TEXT("GameParameter");

const FWwiseMetadataGameParameter* FWwiseRefGameParameter::GetGameParameter() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& GameParameters = SoundBank->GameParameters;
	if (GameParameters.IsValidIndex(GameParameterIndex))
	{
		return &GameParameters[GameParameterIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get GameParameter index #%zu"), GameParameterIndex);
		return nullptr;
	}
}

uint32 FWwiseRefGameParameter::GameParameterId() const
{
	const auto* GameParameter = GetGameParameter();
	if (UNLIKELY(!GameParameter))
	{
		return 0;
	}
	return GameParameter->Id;
}

FGuid FWwiseRefGameParameter::GameParameterGuid() const
{
	const auto* GameParameter = GetGameParameter();
	if (UNLIKELY(!GameParameter))
	{
		return {};
	}
	return GameParameter->GUID;
}

FString FWwiseRefGameParameter::GameParameterName() const
{
	const auto* GameParameter = GetGameParameter();
	if (UNLIKELY(!GameParameter))
	{
		return {};
	}
	return GameParameter->Name;
}

FString FWwiseRefGameParameter::GameParameterObjectPath() const
{
	const auto* GameParameter = GetGameParameter();
	if (UNLIKELY(!GameParameter))
	{
		return {};
	}
	return GameParameter->ObjectPath;
}

uint32 FWwiseRefGameParameter::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(GameParameterIndex));
	return Result;
}
