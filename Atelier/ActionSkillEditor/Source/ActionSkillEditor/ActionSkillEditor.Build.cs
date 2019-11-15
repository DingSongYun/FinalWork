// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ActionSkillEditor : ModuleRules
{
	public ActionSkillEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
                "AdvancedPreviewScene",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
                "UnrealEd",
                "Kismet",
                "KismetWidgets",
				"CommonLibs",
                "ActionSkill",
                "EditorStyle",
                "GraphEditor",
                "AdvancedPreviewScene",
                "Persona",
                "PinnedCommandList",
                "PropertyEditor",
                "InputCore",
				"PropertyEditor",
				"BlueprintGraph",
				"EditorWidgets",
				"SceneOutliner",
				"ApplicationCore",
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
