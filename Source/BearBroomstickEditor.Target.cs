

using UnrealBuildTool;
using System.Collections.Generic;

public class BearBroomstickEditorTarget : TargetRules
{
	public BearBroomstickEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "BearBroomstick" } );
	}
}
