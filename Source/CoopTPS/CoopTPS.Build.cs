// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class CoopTPS : ModuleRules
{
	public CoopTPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivatePCHHeaderFile = "CoopTPS.h";
		PCHUsage = PCHUsageMode.UseSharedPCHs;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", 
        "InputCore", "NavigationSystem", "PhysicsCore", "UMG", "OnlineSubsystemSteam",
        "GameplayCameras","AIModule", "GameplayTasks","NetCore"});

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		PrivateDependencyModuleNames.Add("OnlineSubsystem");
     
        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
