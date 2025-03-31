// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Test_MultiShooter : ModuleRules
{
	public Test_MultiShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "OnlineSubsystemSteam", "OnlineSubsystem", "EnhancedInput" });
	}
}
