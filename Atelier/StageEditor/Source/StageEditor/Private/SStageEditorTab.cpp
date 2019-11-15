#include "SStageEditorTab.h"
#include "Editor.h"
#include "SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Input/SSearchBox.h"
#include "Layout/SSplitter.h"
#include "EditorStyle.h"
#include "EditorUtilities.h"
#include "FileHelpers.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "EditorLevelUtils.h"
#include "LevelUtils.h"
#include "IPluginManager.h"
#include "AssetRegistryModule.h"
#include "SPlacementStageObjectAssetEntry.h"
#include "IStageEditorInterface.h"
#include "SStageEditorTab.h"
#include "StageEditor.h"
#include "StageEditorCommands.h"
#include "StageMode/SStageModeBase.h"
#include "StageMode/SStageListMode.h"
#include "StageMode/SStageEditMode.h"
#include "IStageScene.h"

static const FString STAGE_EDITOR_SCENE_NAME = "StageEditor";

/*------------------------------------------------------------
SStageEditorTab
------------------------------------------------------------*/

SStageEditorTab::~SStageEditorTab()
{
	FEditorDelegates::PostPIEStarted.RemoveAll(this);
	FEditorDelegates::EndPIE.RemoveAll(this);
}

void SStageEditorTab::Construct(const FArguments& InArgs, IStageEditorInterface* InProxy)
{
    this->Proxy = InProxy;

	RegistModes();
	FEditorDelegates::PostPIEStarted.AddRaw(this, &SStageEditorTab::OnBeginPIE);
	FEditorDelegates::EndPIE.AddRaw(this, &SStageEditorTab::OnEndPIE);

	this->ChildSlot
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Fill)
		.VAlign(EVerticalAlignment::VAlign_Fill)
		[
			SNew(SVerticalBox)
			.Visibility_Lambda([&]() { return IsStageEdContextValid() ? EVisibility::Collapsed : EVisibility::Visible; })

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(FText::FromString("Open Stage Editor Scene"))
				.OnClicked(this, &SStageEditorTab::OnClickOpenStageEdLevel)
			]
		]

		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Fill)
		.VAlign(EVerticalAlignment::VAlign_Fill)
		[
			SNew(SVerticalBox)
			.Visibility_Lambda([&]() { return IsStageEdContextValid() ? EVisibility::Visible : EVisibility::Collapsed; })

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SAssignNew(ModeToolBarContainer, SBorder)
					.BorderImage(FEditorStyle::GetBrush("NoBorder"))
					.Padding(FMargin(4, 0))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(BaseToolBarContainer, SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				]
			]

			+ SVerticalBox::Slot()
			[
				SAssignNew(InlineContentHolder, SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			]
		]
	];

	BuildBaseToolBar();
	UpdateModeToolBar();
}

void SStageEditorTab::BuildBaseToolBar()
{
	TSharedPtr<class FUICommandList> StageEditorModesCommands = MakeShareable(new FUICommandList);

	FToolBarBuilder EditorModeTools(StageEditorModesCommands, FMultiBoxCustomization::None);
	{
		EditorModeTools.AddToolBarButton
		(
			FUIAction(FExecuteAction::CreateRaw(this, &SStageEditorTab::SaveStage)),
			FName(),
			FText::FromString("Save"),
			FText::FromString("Save all stage data"),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Save"),
			EUserInterfaceActionType::Button
		);
	}

	BaseToolBarContainer->SetContent(EditorModeTools.MakeWidget());
}

void SStageEditorTab::SaveStage()
{
	FStageEditorModule::Get().SaveStageDataToFile();
}

void SStageEditorTab::RegistModes()
{
	RegisterMode
	(
		SNew(SStageListMode)
		.OnChoosenItem(this, &SStageEditorTab::SetEditingStageScope)
	);
	RegisterMode(SNew(SStageEditMode, Proxy));
}

void SStageEditorTab::RegisterMode(TSharedPtr<SStageModeBase> InMode, bool bInitSelected)
{
	check(InMode.IsValid());

	StageModeList.Add(InMode);
	if (bInitSelected)
	{
		CurrentSelectedStageMode = InMode;
	}
}

void SStageEditorTab::SetEditingStageScope(FStageScopePtr Stage)
{
	EditingStageScopePtr = Stage;
	CurrentSelectedStageMode = StageModeList[1];
	IStageScene::Get()->LoadStage(Stage);
	SStageEditMode* EditMode = StaticCast<SStageEditMode*>(CurrentSelectedStageMode.Get());
	EditMode->OnEditingStageChanged(Stage);
	UpdateSelectionMode(CurrentSelectedStageMode);
}

void SStageEditorTab::UpdateModeToolBar()
{
	TSharedPtr<class FUICommandList> StageEditorModesCommands = MakeShareable(new FUICommandList);

	FToolBarBuilder EditorModeTools(StageEditorModesCommands, FMultiBoxCustomization::None);
	{
		EditorModeTools.SetStyle(&FEditorStyle::Get(), "EditorModesToolbar");
		for (auto Mode : StageModeList)
		{
			EditorModeTools.AddToolBarButton
			(
				FUIAction
				(
					FExecuteAction::CreateRaw(this, &SStageEditorTab::UpdateSelectionMode, Mode),
					FCanExecuteAction(),
					FIsActionChecked::CreateRaw(this, &SStageEditorTab::IsActionChecked, Mode)
				),
				FName(),
				FText::FromString(Mode->GetName()),
				FText::FromString(Mode->GetDescription()),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "ConfigEditor.TabIcon"),
				EUserInterfaceActionType::ToggleButton
			);
		}
	}

	ModeToolBarContainer->SetContent(EditorModeTools.MakeWidget());

	if (!CurrentSelectedStageMode.IsValid() && StageModeList.Num() > 0)
	{
		CurrentSelectedStageMode = StageModeList[0];
	}

	UpdateSelectionMode(CurrentSelectedStageMode);
}

FReply SStageEditorTab::OnClickOpenStageEdLevel()
{
	FString StageEdLevel = "/StageEditor/Maps/" + STAGE_EDITOR_SCENE_NAME;
	FEditorUtilities::OpenLevelByName(StageEdLevel);
	FEditorUtilities::PlayInEditor();
	return FReply::Handled();
}

bool SStageEditorTab::IsStageEdLevelValid()
{
	FString MapName = GetWorld()->GetMapName();
	return  MapName.Equals(STAGE_EDITOR_SCENE_NAME)
		|| MapName.EndsWith(STAGE_EDITOR_SCENE_NAME); // For PIE: UEDPIE_0_StageEditor
}

bool SStageEditorTab::IsStageEdContextValid()
{
	return GetWorld()->IsGameWorld() && IsStageEdLevelValid();
}

UWorld * SStageEditorTab::GetWorld()
{
	FWorldContext* PIEWorldContext = GEditor->GetPIEWorldContext();
	UWorld* World = PIEWorldContext ? PIEWorldContext->World() : GEditor->GetEditorWorldContext().World();
	check(World);
	return World;
}

void SStageEditorTab::OnBeginPIE(const bool bIsSimulating)
{
	if (IsStageEdLevelValid())
	{
		Proxy->InitEditorContext();
	}
}

void SStageEditorTab::OnEndPIE(const bool bIsSimulating)
{
	if (IsStageEdLevelValid())
	{
		Proxy->ExitEditorContext();
	}
}

void SStageEditorTab::UpdateSelectionMode(TSharedPtr<SStageModeBase> SelectionMode)
{
	CurrentSelectedStageMode = SelectionMode;
	if (SelectionMode.IsValid())
	{
		if (InlineContentHolder.IsValid())
		{
			InlineContentHolder->SetContent(SelectionMode.ToSharedRef());
		}
	}
	else
	{
		// Handle when there is no selection mode
	}
}

bool SStageEditorTab::IsActionChecked(TSharedPtr<SStageModeBase> InAction)
{
	return CurrentSelectedStageMode.IsValid() && InAction == CurrentSelectedStageMode;
}

void SStageEditorTab::AddReferencedObjects(FReferenceCollector & Collector)
{
}

