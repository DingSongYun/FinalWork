// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-05
#include "ActionSkillDefineMode.h"
#include "ActionSkillEditor.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#include "Settings/ActionSkillSettings.h"
#include "SSplitter.h"
#include "StructureDetailCustomization.h"
#include "TabFactory/SActionEventStructTab.h"
#include "ActionSkill.h"

#define LOCTEXT_NAMESPACE "ActionSkillEditor"

FActionSkillDefineMode::FActionSkillDefineMode(TSharedRef<class FWorkflowCentricApplication> InHostingApp)
	: FApplicationMode(ActionSkillEditorModes::SkillDeclareMode)
{
	HostingAppPtr = InHostingApp;
	TabLayout = FTabManager::NewLayout("Standalone_ActionSkillDefine_Layout_v1.2")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(InHostingApp->GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetSizeCoefficient(0.2f)
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.7f)
						->SetHideTabWell( true )
						->AddTab( ActionSkillEditorTabs::SkillStructTab, ETabState::OpenedTab )
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.3f)
						->SetHideTabWell( true )
						->AddTab( ActionSkillEditorTabs::SubEventStructTab, ETabState::OpenedTab )
					)
				)
				->Split
				(
					FTabManager::NewSplitter()
					->Split
					(
						FTabManager::NewStack()
						->SetHideTabWell( true )
						->AddTab( ActionSkillEditorTabs::ActionEventStructsTab, ETabState::OpenedTab )
					)
				)
			)
		);

	// Extend Toolbar
	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
		{
			TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingAppPtr.Pin());
			ToolbarBuilder.BeginSection("ActionSkill");
			ToolbarBuilder.AddToolBarButton(
				FUIAction(FExecuteAction::CreateSP(ActionSkillEditorPtr.Get(), &FActionSkillEditor::OnClickNewEventType)),
				NAME_None,
				LOCTEXT("Create_ActionSkill_Label", "New Event"),
				LOCTEXT("Create_ActionSkill_ToolTip", "Create a new event type"),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "Persona.CreateAsset")
			);
			ToolbarBuilder.EndSection();
		}
	));
}

void FActionSkillDefineMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FWorkflowCentricApplication> HostingApp = HostingAppPtr.Pin();
	HostingApp->RegisterTabSpawners(InTabManager.ToSharedRef());
	RegisterTabSpawners(InTabManager.ToSharedRef());
	HostingApp->PushTabFactories(TabFactories);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FActionSkillDefineMode::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	InTabManager->RegisterTabSpawner(ActionSkillEditorTabs::SkillStructTab,
		FOnSpawnTab::CreateSP(this, &FActionSkillDefineMode::SpawnSkillStructureTab))
		// .SetDisplayName( LOCTEXT("ActionSkillEditorTab_SkillStructure", "Skill Structure") )
		.SetGroup(HostingAppPtr.Pin()->GetWorkspaceMenuCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Tabs.Variables"));

	InTabManager->RegisterTabSpawner(ActionSkillEditorTabs::ActionEventStructsTab,
		FOnSpawnTab::CreateSP(this, &FActionSkillDefineMode::SpawnEventStructureTab))
		// .SetDisplayName( LOCTEXT("ActionSkillEditorTab_EventStructure", "Event Structures") )
		.SetGroup(HostingAppPtr.Pin()->GetWorkspaceMenuCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Tabs.Variables"));

	InTabManager->RegisterTabSpawner(ActionSkillEditorTabs::SubEventStructTab,
		FOnSpawnTab::CreateSP(this, &FActionSkillDefineMode::SpawnSubEventStructureTab))
		.SetGroup(HostingAppPtr.Pin()->GetWorkspaceMenuCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Tabs.Variables"));
}

TSharedRef<SDockTab> FActionSkillDefineMode::SpawnSingleStructureTab(const FSpawnTabArgs& Args, UScriptStruct* Struct, FText TabLabel)
{
	check(Struct);
	// AddObjectToSave(Struct);
	TSharedRef<SSplitter> Splitter = SNew(SSplitter)
		.Orientation(Orient_Vertical)
		.PhysicalSplitterHandleSize(10.0f)
		.ResizeMode(ESplitterResizeMode::FixedPosition);

	// Property View
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	{
		DetailsViewArgs.bUpdatesFromSelection = false;
		DetailsViewArgs.bLockable = false;
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		DetailsViewArgs.bHideSelectionTip = true;
		DetailsViewArgs.bShowOptions = false;
	}

	TSharedPtr<class IDetailsView> PropertyView = EditModule.CreateDetailView(DetailsViewArgs);
	{
		FOnGetDetailCustomizationInstance LayoutStructDetails = FOnGetDetailCustomizationInstance::CreateStatic(&FUserDefinedStructureDetails::MakeInstance);
		PropertyView->RegisterInstancedCustomPropertyLayout(UUserDefinedStruct::StaticClass(), LayoutStructDetails);
		PropertyView->SetObject(Struct);
	}

	return SNew(SDockTab)
		.Icon( FEditorStyle::GetBrush("GenericEditor.Tabs.Properties") )
		.Label( TabLabel )
		[
			SNew(SSplitter)
			.Orientation(Orient_Vertical)
			.PhysicalSplitterHandleSize(10.0f)
			.ResizeMode(ESplitterResizeMode::FixedPosition)

			+ SSplitter::Slot()
			.Value(0.25f)
			[
				PropertyView.ToSharedRef()
			]
		];
}

TSharedRef<SDockTab> FActionSkillDefineMode::SpawnSkillStructureTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == ActionSkillEditorTabs::SkillStructTab);
	auto SkillStruct = GetDefault<UActionSkillSettings>()->SkillDefinitionStruct;
	AddObjectToSave(SkillStruct);
	return SpawnSingleStructureTab(Args, SkillStruct, LOCTEXT("ActionSkillEditor", "Skill Structure"));
}

TSharedRef<SDockTab> FActionSkillDefineMode::SpawnSubEventStructureTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == ActionSkillEditorTabs::SubEventStructTab);
	auto SubEventStruct = &UActionSkillTable::GetSubEventStruct();
	AddObjectToSave(SubEventStruct);
	return SpawnSingleStructureTab(Args, SubEventStruct, LOCTEXT("ActionSkillEditor", "SubEvent Structure"));
}

TSharedRef<SDockTab> FActionSkillDefineMode::SpawnEventStructureTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == ActionSkillEditorTabs::ActionEventStructsTab);

	return SNew(SDockTab)
		.Icon( FEditorStyle::GetBrush("GenericEditor.Tabs.Properties") )
		.Label( LOCTEXT("ActionSkillEditor", "Event Structure") )
		[
			SNew(SActionEventStructTab, this)
		];
}

void FActionSkillDefineMode::AddTabFactory(FCreateWorkflowTabFactory FactoryCreator)
{
	if (FactoryCreator.IsBound())
	{
		TabFactories.RegisterFactory(FactoryCreator.Execute(HostingAppPtr.Pin()));
	}
}

void FActionSkillDefineMode::RemoveTabFactory(FName TabFactoryID)
{
	TabFactories.UnregisterFactory(TabFactoryID);
}

void FActionSkillDefineMode::AddObjectToSave(UObject* InObject)
{
	if (InObject)
	{
		ObjectsToSave.AddUnique(InObject);
	}
}
#undef LOCTEXT_NAMESPACE