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

using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;
using System.Reflection;

public class AkAudio : ModuleRules
{
	public AkAudio(ReadOnlyTargetRules Target) : base(Target)
	{
		// If packaging as an Engine plugin, the UBT expects to already have a precompiled plugin available
		// This can be set to true so long as plugin was already precompiled
		bUsePrecompiled = false;
		bPrecompile = false;

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bAllowConfidentialPlatformDefines = true;

		PrivateIncludePathModuleNames.AddRange(new[] { 
			"Settings",
			"UMG",
			"TargetPlatform"
		});

		PublicDependencyModuleNames.AddRange(new[] {
			"UMG",
#if !UE_5_1_OR_LATER
            "PhysX", 
			"APEX",
#endif
            "WwiseFileHandler",
			"WwiseResourceLoader",
			"WwiseSoundEngine"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"AudioMixer",
			"Chaos",
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Json",
			"MovieScene",
			"MovieSceneTracks",
			"NetworkReplayStreaming",
			"PhysicsCore",
			"Projects",
			"Slate",
			"SlateCore",
			"XmlParser",
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"DesktopPlatform",
					"GeometryMode",
					"RenderCore",
					"SharedSettingsWidgets",
					"SourceControl",
					"TargetPlatform",
					"UnrealEd"
				});

			PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"EditorStyle",
				"WwiseProjectDatabase",
				"WwiseResourceCooker",
#if UE_5_0_OR_LATER
				"DeveloperToolSettings",
#endif
			});
		}
		if (Target.bBuildWithEditorOnlyData)
		{
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "WwiseProjectDatabase",
					"WwiseResourceCooker",
                    "WwiseResourceLoader",
                });
		}

		PrivateIncludePaths.Add("AkAudio/Private");
		PrivateIncludePaths.Add("AkAudio/Classes/GTE");
	}
}
