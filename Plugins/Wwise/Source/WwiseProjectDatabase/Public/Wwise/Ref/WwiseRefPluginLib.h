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

#include "Wwise/Ref/WwiseRefPluginInfo.h"

class WWISEPROJECTDATABASE_API FWwiseRefPluginLib : public FWwiseRefPluginInfo
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::PluginLib;
	struct FGlobalIdsMap;

	WwiseRefIndexType PluginLibIndex;

	FWwiseRefPluginLib() :
		PluginLibIndex(INDEX_NONE)
	{}
	FWwiseRefPluginLib(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InPluginIndex) :
		FWwiseRefPluginInfo(InRootMediaRef, InJsonFilePath),
		PluginLibIndex(InPluginIndex)
	{}
	const FWwiseMetadataPluginLib* GetPluginLib() const;

	uint32 PluginLibId() const;
	FString PluginLibName() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefPluginLib& Rhs) const
	{
		return FWwiseRefPluginInfo::operator==(Rhs)
			&& PluginLibIndex == Rhs.PluginLibIndex;
	}
	bool operator!=(const FWwiseRefPluginLib& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefPluginLib::FGlobalIdsMap
{
	WwisePluginLibGlobalIdsMap GlobalIdsMap;
};
