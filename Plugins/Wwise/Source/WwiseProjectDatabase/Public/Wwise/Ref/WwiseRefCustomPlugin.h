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

#include "Wwise/Ref/WwiseRefSoundBank.h"

class WWISEPROJECTDATABASE_API FWwiseRefCustomPlugin : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::CustomPlugin;
	struct FGlobalIdsMap;

	WwiseRefIndexType CustomPluginIndex;

	FWwiseRefCustomPlugin() {}
	FWwiseRefCustomPlugin(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InCustomPluginIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		CustomPluginIndex(InCustomPluginIndex)
	{}
	const FWwiseMetadataPlugin* GetPlugin() const;

	uint32 CustomPluginId() const;
	FGuid CustomPluginGuid() const;
	FString CustomPluginName() const;
	FString CustomPluginObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefCustomPlugin& Rhs) const
	{
		return FWwiseRefSoundBank::operator==(Rhs)
			&& CustomPluginIndex == Rhs.CustomPluginIndex;
	}
	bool operator!=(const FWwiseRefCustomPlugin& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefCustomPlugin::FGlobalIdsMap
{
	WwiseCustomPluginGlobalIdsMap GlobalIdsMap;
};
