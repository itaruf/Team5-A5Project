

using UnrealBuildTool;
using System.Collections.Generic;

public class UEPrototypeEditorTarget : TargetRules
{
	public UEPrototypeEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "UEPrototype" } );
	}
}
