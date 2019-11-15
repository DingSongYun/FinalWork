// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StateSequenceEditor : ModuleRules
{
	public StateSequenceEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
                "StateSequenceEditor/Private",
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Sequencer",
                "EditorStyle",
                "InputCore",
                "Kismet",
                "UnrealEd",
                "ActorSequence",
                "ActorSequenceEditor",
                "StateSequence",
                "MovieScene",
                "MovieSceneTools",
                "MovieSceneTracks",
                "LevelEditor",
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
            }
			);

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetTools",
                "MovieSceneTools",
                "Settings",
                "MovieSceneCaptureDialog",
            }
            );
    }
}
