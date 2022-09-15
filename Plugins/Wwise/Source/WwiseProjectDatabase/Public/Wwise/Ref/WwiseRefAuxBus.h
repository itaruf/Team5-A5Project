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

class WWISEPROJECTDATABASE_API FWwiseRefAuxBus : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::AuxBus;
	struct FGlobalIdsMap;

	WwiseRefIndexType AuxBusIndex;

	FWwiseRefAuxBus() {}
	FWwiseRefAuxBus(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InAuxBusIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		AuxBusIndex(InAuxBusIndex)
	{}
	const FWwiseMetadataBus* GetAuxBus() const;
	void GetAllAuxBusRefs(TSet<const FWwiseRefAuxBus*>& OutAuxBusRefs, const WwiseAuxBusGlobalIdsMap& InGlobalMap) const;

	uint32 AuxBusId() const;
	FGuid AuxBusGuid() const;
	FString AuxBusName() const;
	FString AuxBusObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefAuxBus& Rhs) const
	{
		return FWwiseRefSoundBank::operator ==(Rhs)
			&& AuxBusIndex == Rhs.AuxBusIndex;
	}
	bool operator!=(const FWwiseRefAuxBus& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefAuxBus::FGlobalIdsMap
{
	WwiseAuxBusGlobalIdsMap GlobalIdsMap;
};
