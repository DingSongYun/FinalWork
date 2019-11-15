// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-19
#include "ActionSkillEditorMode.h"
#include "ActionSkillEditor.h"
#include "TabFactory/ActionSkillTabSpawners.h"

#define LOCTEXT_NAMESPACE "ActionSkillEditor"

FActionSkillEditorMode::FActionSkillEditorMode(TSharedRef<class FWorkflowCentricApplication> InHostingApp)
	: FApplicationMode(ActionSkillEditorModes::SkillEditorMode)
{
	HostingAppPtr = InHostingApp;

	TabSpawners::RegisterActionSkillTabs(InHostingApp, TabFactories);

	// Layout
	TabLayout = FTabManager::NewLayout("Standalone_ActionSkillEditor_Layout_v1.2")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(HostingAppPtr.Pin()->GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetSizeCoefficient(0.9f)
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
					->SetSizeCoefficient(0.2f)
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->SetHideTabWell(false)
						->AddTab(ActionSkillEditorTabs::SkillPalletteTab, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->SetHideTabWell(false)
						->AddTab(ActionSkillEditorTabs::ActionDetailTab, ETabState::OpenedTab)
					)
				)
				->Split
				(
					FTabManager::NewSplitter()
					->SetSizeCoefficient(0.6f)
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->SetHideTabWell(true)
						->AddTab(ActionSkillEditorTabs::ViewportTab, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->SetHideTabWell(true)
						->AddTab(ActionSkillEditorTabs::OperatingTab, ETabState::OpenedTab)
					)
				)
				->Split
				(
					FTabManager::NewSplitter()
					->SetSizeCoefficient(0.2f)
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewSplitter()
						->SetSizeCoefficient(0.4f)
						->SetOrientation(Orient_Horizontal)
						->Split
						(
							FTabManager::NewStack()
							->SetSizeCoefficient(0.4f)
							->SetHideTabWell(false)
							->AddTab(ActionSkillEditorTabs::PrevSceneSettingTab, ETabState::OpenedTab)
						)
						->Split
						(
							FTabManager::NewStack()
							->SetSizeCoefficient(0.4f)
							->SetHideTabWell(false)
							->AddTab(ActionSkillEditorTabs::WorldOutlineTab, ETabState::OpenedTab)
						)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->SetHideTabWell(false)
						->AddTab(ActionSkillEditorTabs::EventDetailTab, ETabState::OpenedTab)
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
				FUIAction(FExecuteAction::CreateSP(ActionSkillEditorPtr.Get(), &FActionSkillEditor::OnClickCreateSkill)),
				NAME_None,
				LOCTEXT("Create_ActionSkill_Label", "Create Skill"),
				LOCTEXT("Create_ActionSkill_ToolTip", "Create a new skill"),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "Persona.CreateAsset")
			);
			ToolbarBuilder.EndSection();
		}
	));
}

void FActionSkillEditorMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FWorkflowCentricApplication> HostingApp = HostingAppPtr.Pin();
	HostingApp->RegisterTabSpawners(InTabManager.ToSharedRef());
	HostingApp->PushTabFactories(TabFactories);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FActionSkillEditorMode::AddTabFactory(FCreateWorkflowTabFactory FactoryCreator)
{
	if (FactoryCreator.IsBound())
	{
		TabFactories.RegisterFactory(FactoryCreator.Execute(HostingAppPtr.Pin()));
	}
}

void FActionSkillEditorMode::RemoveTabFactory(FName TabFactoryID)
{
	TabFactories.UnregisterFactory(TabFactoryID);
}

void FActionSkillEditorMode::PreDeactivateMode()
{
	// Notify editor mode changed
	TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingAppPtr.Pin());
	ActionSkillEditorPtr->OnPreEditorModeDeactivated();

	FApplicationMode::PreDeactivateMode();
}

#undef LOCTEXT_NAMESPACE