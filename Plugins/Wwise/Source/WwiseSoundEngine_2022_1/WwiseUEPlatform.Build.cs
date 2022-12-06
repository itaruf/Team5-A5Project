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

// Platform-specific files implement this interface, returning their particular dependencies, defines, etc.
public abstract class WwiseUEPlatform
{
	protected ReadOnlyTargetRules Target;
	protected string ThirdPartyFolder;

	public WwiseUEPlatform(ReadOnlyTargetRules in_Target, string in_ThirdPartyFolder)
	{
		Target = in_Target;
		ThirdPartyFolder = in_ThirdPartyFolder;
	}

	public static WwiseUEPlatform GetWwiseUEPlatformInstance(ReadOnlyTargetRules Target, string ThirdPartyFolder)
	{
		var WwiseUEPlatformType = System.Type.GetType("WwiseUEPlatform_" + Target.Platform.ToString());
		if (WwiseUEPlatformType == null)
		{
			throw new BuildException("Wwise does not support platform " + Target.Platform.ToString());
		}

		var PlatformInstance = Activator.CreateInstance(WwiseUEPlatformType, Target, ThirdPartyFolder) as WwiseUEPlatform;
		if (PlatformInstance == null)
		{
			throw new BuildException("Wwise could not instantiate platform " + Target.Platform.ToString());
		}

		return PlatformInstance;
	}

	protected static List<string> GetAllLibrariesInFolder(string LibFolder, string Extension, bool RemoveLibPrefix = true, bool GetFullPath = false)
	{
		List<string> ret = null;
		var FoundLibs = Directory.GetFiles(LibFolder, "*."+Extension);

		if (GetFullPath)
		{
			ret = new List<string>(FoundLibs);
		}
		else
		{
			ret = new List<string>();
			foreach (var Library in FoundLibs)
			{
				var LibName = Path.GetFileNameWithoutExtension(Library);
				if (RemoveLibPrefix && LibName.StartsWith("lib"))
				{
					LibName = LibName.Remove(0, 3);
				}
				ret.Add(LibName);
			}

		}
		return ret;
	}

	public virtual string AkConfigurationDir
	{
		get
		{
			switch (Target.Configuration)
			{
				case UnrealTargetConfiguration.Debug:
					var akConfiguration = Target.bDebugBuildsActuallyUseDebugCRT ? "Debug" : "Profile";
					return akConfiguration;

				case UnrealTargetConfiguration.Development:
				case UnrealTargetConfiguration.Test:
				case UnrealTargetConfiguration.DebugGame:
					return "Profile";
				default:
					return "Release";
			}
		}
	}
	
	public abstract string GetLibraryFullPath(string LibName, string LibPath);
	public abstract bool SupportsAkAutobahn { get; }
	public abstract bool SupportsCommunication { get; }
	public abstract bool SupportsDeviceMemory { get; }
	public abstract string AkPlatformLibDir { get; }
	public abstract string DynamicLibExtension { get; }
	public virtual bool SupportsOpus { get { return true; } }

	public virtual List<string> GetPublicLibraryPaths()
	{
		return new List<string>
		{
			Path.Combine(ThirdPartyFolder, AkPlatformLibDir, AkConfigurationDir, "lib")
		};
	}

	public virtual List<string> GetRuntimeDependencies()
	{
		return GetAllLibrariesInFolder(Path.Combine(ThirdPartyFolder, AkPlatformLibDir, AkConfigurationDir, "bin"), DynamicLibExtension, false, true);
	}

	public abstract List<string> GetAdditionalWwiseLibs();
	public abstract List<string> GetPublicSystemLibraries();
	public abstract List<string> GetPublicDelayLoadDLLs();
	public abstract List<string> GetPublicDefinitions();
	public abstract Tuple<string, string> GetAdditionalPropertyForReceipt(string ModuleDirectory);
	public abstract List<string> GetPublicFrameworks();
	
	public virtual List<string> GetSanitizedAkLibList(List<string> AkLibs)
	{
		List<string> SanitizedLibs = new List<string>();
		foreach(var lib in AkLibs)
		{
			foreach(var libPath in GetPublicLibraryPaths())
			{
				SanitizedLibs.Add(GetLibraryFullPath(lib, libPath));
			}
		}
		
		return SanitizedLibs;
	}
}
