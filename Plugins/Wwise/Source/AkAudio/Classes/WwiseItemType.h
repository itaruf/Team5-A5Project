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

#pragma once

#include "Containers/UnrealString.h"
#include "Engine/EngineTypes.h"

namespace EWwiseItemType
{
	UENUM()
	enum Type
	{
		Event,
		AuxBus,
		AcousticTexture,
		State,
		Switch,
		GameParameter,
		Trigger,
		EffectShareSet,
		ActorMixer,
		Bus,
		Project,
		StandaloneWorkUnit,
		NestedWorkUnit,
		PhysicalFolder,
		Folder,
		Sound,
		SwitchContainer,
		RandomSequenceContainer,
		BlendContainer,
		MotionBus,
		StateGroup,
		SwitchGroup,

		LastWwisePickerType = EffectShareSet,
		LastWaapiPickerType = ActorMixer,

		None = -1,
	};

	//Name to show in the Picker
	static const FString PickerDisplayNames[] = {
		TEXT("Event"),
		TEXT("AuxBus"),
		TEXT("AcousticTexture"),
		TEXT("State"),
		TEXT("Switch"),
		TEXT("GameParameter"),
		TEXT("Trigger"),
		TEXT("Effect ShareSets"),
		TEXT("Actor-Mixer Hierarchy"),
	};

	//Tag in the work unit XML for this WwiseObjectType
	static const FString WorkUnitTagNames[] = {
		TEXT("Events"),
		TEXT("Busses"),
		TEXT("VirtualAcoustics"),
		TEXT("States"),
		TEXT("Switches"),
		TEXT("GameParameters"),
		TEXT("Triggers"),
		TEXT("Effects"),
		TEXT("ActorMixer"),
	};

	//Name of the folder containing the work units of this WwiseObjectType
	static const FString FolderNames[] = {
		TEXT("Events"),
		TEXT("Master-Mixer Hierarchy"),
		TEXT("Virtual Acoustics"),
		TEXT("States"),
		TEXT("Switches"),
		TEXT("Game Parameters"),
		TEXT("Triggers"),
		TEXT("Effects"),
		TEXT("Actor-Mixer Hierarchy"),
	};

	static const TArray<FString> PhysicalFoldersToIgnore = {
		TEXT("Actor-Mixer Hierarchy"),
		TEXT("Attenuations"),
		TEXT("Audio Devices"),
		TEXT("Control Surface Sessions"),
		TEXT("Conversion Settings"),
		TEXT("Dynamic Dialogue"),
		TEXT("Interactive Music Hierarchy"),
		TEXT("Metadata"),
		TEXT("Mixing Sessions"),
		TEXT("Modulators"),
		TEXT("Presets"),
		TEXT("Queries"),
		TEXT("SoundBanks"),
		TEXT("Soundcaster Sessions"),
	};

	inline Type FromString(const FString& ItemName)
	{
		struct TypePair
		{
			FString Name;
			Type Value;
		};

		static const TypePair ValidTypes[] = {
			{TEXT("AcousticTexture"), Type::AcousticTexture},
			{TEXT("ActorMixer"), Type::ActorMixer},
			{TEXT("AuxBus"), Type::AuxBus},
			{TEXT("BlendContainer"), Type::BlendContainer},
			{TEXT("Bus"), Type::Bus},
			{TEXT("Event"), Type::Event},
			{TEXT("Folder"), Type::Folder},
			{TEXT("GameParameter"), Type::GameParameter},
			{TEXT("MotionBus"), Type::MotionBus},
			{TEXT("PhysicalFolder"), Type::PhysicalFolder},
			{TEXT("Project"), Type::Project},
			{TEXT("RandomSequenceContainer"), Type::RandomSequenceContainer},
			{TEXT("Sound"), Type::Sound},
			{TEXT("State"), Type::State},
			{TEXT("StateGroup"), Type::StateGroup},
			{TEXT("Switch"), Type::Switch},
			{TEXT("SwitchContainer"), Type::SwitchContainer},
			{TEXT("SwitchGroup"), Type::SwitchGroup},
			{TEXT("Trigger"), Type::Trigger},
			{TEXT("WorkUnit"), Type::StandaloneWorkUnit},
			{TEXT("Effect"), Type::EffectShareSet},

		};

		for (const auto& type : ValidTypes)
		{
			if (type.Name == ItemName)
			{
				return type.Value;
			}
		}

		return Type::None;
	}

	inline Type FromFolderName(const FString& ItemName)
	{
		struct TypePair
		{
			FString Name;
			Type Value;
		};

		static const TypePair ValidTypes[] = {
			{TEXT("Virtual Acoustics"), Type::AcousticTexture},
			{TEXT("Actor-Mixer Hierarchy"), Type::ActorMixer},
			{TEXT("Master-Mixer Hierarchy"), Type::AuxBus},
			{TEXT("Events"), Type::Event},
			{TEXT("Game Parameters"), Type::GameParameter},
			{TEXT("States"), Type::State},
			{TEXT("Switches"), Type::Switch},
			{TEXT("Triggers"), Type::Trigger},
			{TEXT("Effects"), Type::EffectShareSet},
		};

		for (const auto& type : ValidTypes)
		{
			if (type.Name == ItemName)
			{
				return type.Value;
			}
		}

		return Type::None;
	}
};
