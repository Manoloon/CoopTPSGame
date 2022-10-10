// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class CoopTpsClientTarget : TargetRules
{
	public CoopTpsClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ShadowVariableWarningLevel = WarningLevel.Warning;
		ExtraModuleNames.AddRange( new string[] { "CoopTPS" } );
	}
}
