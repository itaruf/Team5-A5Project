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
			"PhysX", 
			"APEX",
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
