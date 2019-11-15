// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class AtelierEditor: ModuleRules
{
	public AtelierEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "JsonUtilities", "Json", "NavigationSystem", "AIModule" });
		PublicDependencyModuleNames.AddRange(new string[] { "Atelier" });
		PublicDependencyModuleNames.AddRange(new string[] { "GraphEditor", "AnimGraph", "AnimationCore", "AnimGraphRuntime", "BlueprintGraph", "Navmesh", "UMGEditor", "PropertyEditor"});

		PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
		PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "Slate",
            "SlateCore",
            "EditorStyle",
            "AnimGraph",
            "Paper2D",
            "UMG",
            "slua_unreal",
            "AkAudio",
            "Persona",
            "ApplicationCore",

            /* Skill Editor */
            "ActionSkillEditor",
            "StageEditor",
            "slua_unreal",
        });

		PublicIncludePaths.AddRange(new string[] { "AtelierEditor/Public" } );
		PrivateIncludePaths.AddRange(new string[] { "AtelierEditor/Private" } );

        if (Target.bCompileRecast)
        {
            PrivateDependencyModuleNames.Add("Navmesh");
            PublicDefinitions.Add("WITH_RECAST=1");
        }
	}
}
