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

using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;
using System.Reflection;

public class WwiseSoundEngine : ModuleRules
{
	private static WwiseUEPlatform WwiseUEPlatformInstance;
	private List<string> AkLibs = new List<string> 
	{
		"AkSoundEngine",
		"AkMemoryMgr",
		"AkStreamMgr",
		"AkMusicEngine",
		"AkSpatialAudio",
		"AkAudioInputSource",
		"AkVorbisDecoder",
		"AkMeterFX", // AkMeter does not have a dedicated DLL
	};
	
	public WwiseSoundEngine(ReadOnlyTargetRules Target) : base(Target)
	{
		// If packaging as an Engine plugin, the UBT expects to already have a precompiled plugin available
		// This can be set to true so long as plugin was already precompiled
		bUsePrecompiled = false;
		bPrecompile = false;

		string ThirdPartyFolder = Path.Combine(ModuleDirectory, "../../ThirdParty");
		WwiseUEPlatformInstance = WwiseUEPlatform.GetWwiseUEPlatformInstance(Target, ThirdPartyFolder);
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bAllowConfidentialPlatformDefines = true;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		if (Target.bBuildEditor)
		{
			foreach (var Platform in GetAvailablePlatforms(ModuleDirectory))
			{
				PublicDefinitions.Add("AK_PLATFORM_" + Platform.ToUpper());
			}
		}

        PublicIncludePaths.Add(Path.Combine(ThirdPartyFolder, "include"));

		PublicDefinitions.Add("AK_UNREAL_MAX_CONCURRENT_IO=32");
		PublicDefinitions.Add("AK_UNREAL_IO_GRANULARITY=32768");
		if (Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			PublicDefinitions.Add("AK_OPTIMIZED");
		}

		if (Target.Configuration != UnrealTargetConfiguration.Shipping && WwiseUEPlatformInstance.SupportsCommunication)
		{
			AkLibs.Add("CommunicationCentral");
			PublicDefinitions.Add("AK_ENABLE_COMMUNICATION=1");
		}
		else
		{
			PublicDefinitions.Add("AK_ENABLE_COMMUNICATION=0");
		}

		if (WwiseUEPlatformInstance.SupportsAkAutobahn)
		{
			AkLibs.Add("AkAutobahn");
			PublicDefinitions.Add("AK_SUPPORT_WAAPI=1");
		}
		else
		{
			PublicDefinitions.Add("AK_SUPPORT_WAAPI=0");
		}

		if (WwiseUEPlatformInstance.SupportsOpus)
		{
			AkLibs.Add("AkOpusDecoder");
			PublicDefinitions.Add("AK_SUPPORT_OPUS=1");
		}
		else
		{
			PublicDefinitions.Add("AK_SUPPORT_OPUS=0");
		}

		if (WwiseUEPlatformInstance.SupportsDeviceMemory)
		{
			PublicDefinitions.Add("AK_SUPPORT_DEVICE_MEMORY=1");
		}
		else
		{
			PublicDefinitions.Add("AK_SUPPORT_DEVICE_MEMORY=0");
		}

		// Platform-specific dependencies
		PublicSystemLibraries.AddRange(WwiseUEPlatformInstance.GetPublicSystemLibraries());
		AkLibs.AddRange(WwiseUEPlatformInstance.GetAdditionalWwiseLibs());
		PublicDefinitions.AddRange(WwiseUEPlatformInstance.GetPublicDefinitions());
		PublicDefinitions.Add(string.Format("AK_CONFIGURATION=\"{0}\"", WwiseUEPlatformInstance.AkConfigurationDir));
		var AdditionalProperty = WwiseUEPlatformInstance.GetAdditionalPropertyForReceipt(ModuleDirectory);
		if (AdditionalProperty != null)
		{
			AdditionalPropertiesForReceipt.Add(AdditionalProperty.Item1, AdditionalProperty.Item2);
		}

		PublicFrameworks.AddRange(WwiseUEPlatformInstance.GetPublicFrameworks());
		
		PublicDelayLoadDLLs.AddRange(WwiseUEPlatformInstance.GetPublicDelayLoadDLLs());
		foreach(var RuntimeDependency in WwiseUEPlatformInstance.GetRuntimeDependencies())
		{
			RuntimeDependencies.Add(RuntimeDependency);
		}
		PublicAdditionalLibraries.AddRange(WwiseUEPlatformInstance.GetSanitizedAkLibList(AkLibs));
	}

	private static List<string> GetAvailablePlatforms(string ModuleDir)
	{
		var FoundPlatforms = new List<string>();
		const string StartPattern = "WwiseUEPlatform_";
		const string EndPattern = ".Build.cs";
		foreach (var BuildCsFile in System.IO.Directory.GetFiles(ModuleDir, "*" + EndPattern))
		{
			if (BuildCsFile.Contains("WwiseUEPlatform_"))
			{
				int StartIndex = BuildCsFile.IndexOf(StartPattern) + StartPattern.Length;
				int StopIndex = BuildCsFile.IndexOf(EndPattern);
				FoundPlatforms.Add(BuildCsFile.Substring(StartIndex, StopIndex - StartIndex));
			}
		}

		return FoundPlatforms;
	}
}
