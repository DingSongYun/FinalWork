// Coypright

#include "AtelierEditor.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "EditorMenuExtensions.h"
#include "MapEditMode.h"
#include "SMapEditorToolbar.h"
#include "SAccessoriesEditorToolbar.h"
#include "SWorkshopEditorToolbar.h"
#include "SFurnitureEditorToolbar.h"
#include "SInteractiveEditorToolbar.h"
#include "SAvatarEditorToolbar.h"
#include "SDockTab.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructure.h"
#include "LevelEditor.h"
#include "PropertyEditorModule.h"
#include "Stage/StateOfDetailComponent.h"
#include "SODComponentDetailsCustomization.h"

#include "PropertyEditorModule.h"
#include "AnimGraphNode_SlotBoneBlend.h"
#include "AnimGraphNode_BSSlot.h"
#include "CustomizeDetails/AnimGraphNodeSlotDetails.h"
#include "Components/BrushComponent.h"
#include "CustomizeDetails/ResizableBrushComponentVisualizer.h"
#include "ActionSkillEditorModule.h"
#include "ActionSkill/SkillEditorMiddleware.h"

#include "StageEditor.h"
#include "StageEditor/StageEditorProxy.h"

IMPLEMENT_GAME_MODULE(FAtelierEditorModule, AtelierEditor);

TSharedRef<SDockTab> FAtelierEditorModule::CreateMapEditorTab(const FSpawnTabArgs& Args)
{
	TArray<FEditorTool*> ToolsToRegister;
	for (const auto& EditorTool: EditorTools)
	{
		check(EditorTool.Get() != nullptr);
		ToolsToRegister.Add(EditorTool.Get());
	}

	TSharedRef<SMapEditorToolbar> tpMapEditorToolbar =
		SNew(SMapEditorToolbar).ToolsToRegister(ToolsToRegister);

	// Construct dock tab
	TSharedRef<SDockTab> DockTab =
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			tpMapEditorToolbar
		];	

	this->MapEditorToolbar = tpMapEditorToolbar;
	return DockTab;
}

TSharedRef<SDockTab> FAtelierEditorModule::CreateAccessoriesEditorTab(const FSpawnTabArgs & Args)
{
	TArray<FEditorTool*> ToolsToRegister;
	for (const auto& EditorTool : EditorTools)
	{
		check(EditorTool.Get() != nullptr);
		ToolsToRegister.Add(EditorTool.Get());
	}

	TSharedRef<SAccessoriesEditorToolbar> tpAccessoriesEditorToolbar =
		SNew(SAccessoriesEditorToolbar).ToolsToRegister(ToolsToRegister);

	// Construct dock tab
	TSharedRef<SDockTab> DockTab =
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			tpAccessoriesEditorToolbar
		];

	this->AccessoriesEditorToolbar = tpAccessoriesEditorToolbar;
	return DockTab;
}

TSharedRef<SDockTab> FAtelierEditorModule::CreateInteractiveEditorTab(const FSpawnTabArgs & Args)
{
	TArray<FEditorTool*> ToolsToRegister;
	for (const auto& EditorTool : EditorTools)
	{
		check(EditorTool.Get() != nullptr);
		ToolsToRegister.Add(EditorTool.Get());
	}

	TSharedRef<SInteractiveEditorToolbar> tpEditorToolbar =
		SNew(SInteractiveEditorToolbar).ToolsToRegister(ToolsToRegister);

	// Construct dock tab
	TSharedRef<SDockTab> DockTab =
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			tpEditorToolbar
		];

	this->InteractiveEditorToolbar = tpEditorToolbar;
	return DockTab;
}


TSharedRef<SDockTab> FAtelierEditorModule::CreateAvatarEditorTab(const FSpawnTabArgs & Args)
{
	TArray<FEditorTool*> ToolsToRegister;
	for (const auto& EditorTool : EditorTools)
	{
		check(EditorTool.Get() != nullptr);
		ToolsToRegister.Add(EditorTool.Get());
	}

	TSharedRef<SAvatarEditorToolbar> tpAvatarEditorToolbar =
		SNew(SAvatarEditorToolbar).ToolsToRegister(ToolsToRegister);

	// Construct dock tab
	TSharedRef<SDockTab> DockTab =
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			tpAvatarEditorToolbar
		];

	this->AvatarEditorToolbar = tpAvatarEditorToolbar;
	return DockTab;
}

TSharedRef<SDockTab> FAtelierEditorModule::CreateWorkshopEditorTab(const FSpawnTabArgs & Args)
{
	TArray<FEditorTool*> ToolsToRegister;
	for (const auto& EditorTool : EditorTools)
	{
		check(EditorTool.Get() != nullptr);
		ToolsToRegister.Add(EditorTool.Get());
	}

	TSharedRef<SWorkshopEditorToolbar> EditorToolbar =
		SNew(SWorkshopEditorToolbar).ToolsToRegister(ToolsToRegister);

	// Construct dock tab
	TSharedRef<SDockTab> DockTab =
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			EditorToolbar
		];

	this->WorkshopEditorToolbar = EditorToolbar;
	return DockTab;
}

TSharedRef<SDockTab> FAtelierEditorModule::CreateFurnitureEditorTab(const FSpawnTabArgs & Args)
{
	TArray<FEditorTool*> ToolsToRegister;
	for (const auto& EditorTool : EditorTools)
	{
		check(EditorTool.Get() != nullptr);
		ToolsToRegister.Add(EditorTool.Get());
	}

	TSharedRef<SFurnitureEditorToolbar> EditorToolbar =
		SNew(SFurnitureEditorToolbar).ToolsToRegister(ToolsToRegister);

	// Construct dock tab
	TSharedRef<SDockTab> DockTab =
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			EditorToolbar
		];

	this->FurnitureEditorToolbar = EditorToolbar;
	return DockTab;
}
void FAtelierEditorModule::StartupModule()
{
	// Load Modules
	FModuleManager::Get().LoadModule(TEXT("StageEditor"));

	Config = NewObject<UEditorConfig>();
	Config->SaveConfig();

	// This is no need to add Menus currently
	//FEditorMenuExtensions::SetupMenus();

	// Add Toolbars 
	FEditorMenuExtensions::ExtendToolbars();
	FEditorMenuExtensions::ExtendContexMenu();
	FEditorMenuExtensions::ExtendContentBrowserContextMenu();

	FTabSpawnerEntry MapEditorEntry = FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(MapEditorTabName, FOnSpawnTab::CreateRaw(this, &FAtelierEditorModule::CreateMapEditorTab))
		.SetDisplayName(FText::FromString("MapEditor"))
		.SetTooltipText(FText::FromString("Open Map Editor Tab"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "MergeActors.TabIcon"));

	FTabSpawnerEntry AccessoriesEntry = FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(AccessoriesEditorTabName, FOnSpawnTab::CreateRaw(this, &FAtelierEditorModule::CreateAccessoriesEditorTab))
		.SetDisplayName(FText::FromString("AccessoriesEditor"))
		.SetTooltipText(FText::FromString("Open Accessories Editor Tab"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "MergeActors.TabIcon"));

	FTabSpawnerEntry InteractiveEntry = FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(InteractiveEditorTabName, FOnSpawnTab::CreateRaw(this, &FAtelierEditorModule::CreateInteractiveEditorTab))
		.SetDisplayName(FText::FromString("InteractiveEditor"))
		.SetTooltipText(FText::FromString("Open Interactive Editor Tab"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "MergeActors.TabIcon"));

	FTabSpawnerEntry AvatarEditorEntry = FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(FName("AvatarEditor"), FOnSpawnTab::CreateRaw(this, &FAtelierEditorModule::CreateAvatarEditorTab))
		.SetDisplayName(FText::FromString("AvatarEditor"))
		.SetTooltipText(FText::FromString("Open Avatar Editor Tab"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "MergeActors.TabIcon"));

	FTabSpawnerEntry WorkshopEditorEntry = FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(FName("WorkshopEditor"), FOnSpawnTab::CreateRaw(this, &FAtelierEditorModule::CreateWorkshopEditorTab))
		.SetDisplayName(FText::FromString("WorkshopEditor"))
		.SetTooltipText(FText::FromString("Open Workshop Editor Tab"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "MergeActors.TabIcon"));

	FTabSpawnerEntry FurnitureEditorEntry = FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(FName("FurnitureEditor"), FOnSpawnTab::CreateRaw(this, &FAtelierEditorModule::CreateFurnitureEditorTab))
		.SetDisplayName(FText::FromString("FurnitureEditor"))
		.SetTooltipText(FText::FromString("Open Furniture Editor Tab"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "MergeActors.TabIcon"));
	this->RegisterCustomDetail();


	IActionSkillEditorModule& ActionSkillModule = FModuleManager::GetModuleChecked<IActionSkillEditorModule>(TEXT("ActionSkillEditor"));
	{
		ActionSkillModule.OnOpenEditorDelegate().AddStatic(&FSkillEditorMiddleware::OnCreateEditor);
		ActionSkillModule.OnEditorInitializedDelegate().AddStatic(&FSkillEditorMiddleware::OnInitializeEditor);
		ActionSkillModule.OnCloseEditorDelegate().AddStatic(&FSkillEditorMiddleware::OnExitEditor);
		ActionSkillModule.SetSkillPreviewProxy(
			IActionSkillEditorModule::FGetSkillPreviewProxyDelegate::CreateStatic(
				&FSkillEditorMiddleware::GetSkillPreviewProxy)
		);
	}

	FStageEditorModule& StageEditorModule = FModuleManager::GetModuleChecked<FStageEditorModule>(TEXT("StageEditor"));
	{
		StageEditorModule.RegisterProxyInteface<FStageEditorProxy>();
	}
}

void FAtelierEditorModule::ShutdownModule()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MapEditorTabName);
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AccessoriesEditorTabName);
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(InteractiveEditorTabName);
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AvatarEditor"));
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(WorkshopEditorTabName);
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FurnitureEditorTabName);
	}

	if(GUnrealEd != NULL)
	{
		// Iterate over all class names we registered for
		for(FName ClassName : RegisteredComponentClassNames)
		{
			GUnrealEd->UnregisterComponentVisualizer(ClassName);
		}
	}
}

void FAtelierEditorModule::RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer)
{
	if (GUnrealEd != NULL)
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
	}

	RegisteredComponentClassNames.Add(ComponentClassName);

	if (Visualizer.IsValid())
	{
		Visualizer->OnRegister();
	}
}

void FAtelierEditorModule::RegisterCustomDetail()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UStateOfDetailComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FSODComponentDetailsCustomizaiton::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UAnimGraphNode_SlotBoneBlend::StaticClass()->GetFName(), 
		FOnGetDetailCustomizationInstance::CreateStatic(&FAnimGraphNodeSlotBlendDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UAnimGraphNode_BSSlot::StaticClass()->GetFName(), 
		FOnGetDetailCustomizationInstance::CreateStatic(&FAnimGraphNodeSlotDetails::MakeInstance));

	RegisterComponentVisualizer(UBrushComponent::StaticClass()->GetFName(), MakeShareable(new FResizableBrushComponentVisualizer));
}
