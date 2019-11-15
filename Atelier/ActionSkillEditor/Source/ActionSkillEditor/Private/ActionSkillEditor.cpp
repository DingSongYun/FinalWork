// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-19

#include "ActionSkillEditor.h"
#include "ActionSkillEditorModule.h"
#include "ActionSkillEditorPreviewScene.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "TabFactory/SActionSkillPalette.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Toolkits/IToolkitHost.h"
#include "ActionSkillEditorMode.h"
#include "ActionSkillDefineMode.h"
#include "Toolkits/IToolkit.h"
#include "EditorStyleSet.h"
#include "SlateOptMacros.h"
#include "LevelEditor.h"
#include "ActionSkill.h"
#include "SlateCore.h"
#include "Editor.h"
#include "ActionSkillTableCSV.h"
#include "Settings/ActionSkillSettings.h"
#include "Settings/ActionSkillSettings.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "WorkflowOrientedApp/SModeWidget.h"
#include "FileHelpers.h"
#include "ActionSkillEditorModule.h"
#include "GameFramework/WorldSettings.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "ISkillPreviewProxy.h"
#include "Factories/ASTypeFactory.h"

const FName ACTIONSKILL_EDITOR_NAME = FName(TEXT("ActionSkillEditor"));
const FName ACTIONSKILL_EDITOR_APP_IDENTIFIER = FName(TEXT("ActionSkillEditorApp"));
const FName ActionSkillEditorModes::SkillEditorMode(TEXT("SkillEditorMode"));
const FName ActionSkillEditorModes::SkillDeclareMode(TEXT("SkillDeclareMode"));

/* Tabs */
const FName ActionSkillEditorTabs::SkillPalletteTab(TEXT("SkillPallette"));
const FName ActionSkillEditorTabs::EventPalletteTab(TEXT("EventPallette"));
const FName ActionSkillEditorTabs::ViewportTab(TEXT("ViewportTab"));
const FName ActionSkillEditorTabs::OperatingTab(TEXT("OperatingTab"));
const FName ActionSkillEditorTabs::ActionDetailTab(TEXT("DetailTab"));
const FName ActionSkillEditorTabs::EventDetailTab(TEXT("EventTab"));
const FName ActionSkillEditorTabs::SkillStructTab(TEXT("SkillStructTab"));
const FName ActionSkillEditorTabs::SubEventStructTab(TEXT("SubEventStructTab"));
const FName ActionSkillEditorTabs::ActionEventStructsTab(TEXT("ActionEventStructsTab"));
const FName ActionSkillEditorTabs::PrevSceneSettingTab(TEXT("PrevSceneSetting"));
const FName ActionSkillEditorTabs::WorldOutlineTab(TEXT("WorldOutlineTab"));
const FName ActionSkillEditorTabs::AssetBrowserTab(TEXT("AssetBrowserTab"));

DEFINE_LOG_CATEGORY(LogActionSkillEditor);

#define LOCTEXT_NAMESPACE "ActionSkillEditor"

FActionSkillEditor::FActionSkillEditor()
    : EditingActionKeyFrame(nullptr)
{
    if (UEditorEngine* Editor = Cast<UEditorEngine>(GEngine))
    {
        Editor->RegisterForUndo(this);
    }

    FModuleManager::LoadModuleChecked<IActionSkillEditorModule>("ActionSkillEditor")
        .OnOpenEditorDelegate()
        .Broadcast();
}

FActionSkillEditor::~FActionSkillEditor()
{
    if (UEditorEngine* Editor = Cast<UEditorEngine>(GEngine))
    {
        Editor->UnregisterForUndo(this);
    }
}

/**
 * 编辑器初始化
 */
void FActionSkillEditor::InitializeEditor()
{
    CreatePreviewScene();
    const EToolkitMode::Type Mode = EToolkitMode::Standalone;

    TSharedPtr<IToolkitHost> InitToolkitHost = Mode == EToolkitMode::WorldCentric
        ? FModuleManager::LoadModuleChecked< FLevelEditorModule >("LevelEditor").GetFirstLevelEditor()
        : TSharedPtr<IToolkitHost>();

    // 加载技能数据
    LoadSkillTable();

    // 初始化资源编辑器
    const bool bCreateDefaultStandaloneMenu = true;
    const bool bCreateDefaultToolbar = true;
    const TSharedRef<FTabManager::FLayout> DummyLayout = FTabManager::NewLayout("NullLayout")->AddArea(FTabManager::NewPrimaryArea());
    FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, ACTIONSKILL_EDITOR_APP_IDENTIFIER, 
        DummyLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, FActionSkillModule::Get().GetSkillTable());

    // FWorkflowCentricApplication
    AddApplicationMode(
        ActionSkillEditorModes::SkillEditorMode,
        MakeShareable(new FActionSkillEditorMode(SharedThis(this)))
    );

    AddApplicationMode(
        ActionSkillEditorModes::SkillDeclareMode,
        MakeShareable(new FActionSkillDefineMode(SharedThis(this)))
    );

    SetCurrentMode(ActionSkillEditorModes::SkillEditorMode);

    // 工具栏
    ExtendToolbar();

    // 创建相关Menu和Toolbar
    RegenerateMenusAndToolbars();

    FModuleManager::LoadModuleChecked<IActionSkillEditorModule>("ActionSkillEditor")
        .OnEditorInitializedDelegate().Broadcast();
    
    GetSkillPreviewProxy()->OnInitialized();
}

bool FActionSkillEditor::OnRequestClose()
{
    GetSkillPreviewProxy()->OnTerminated();
    FModuleManager::LoadModuleChecked<IActionSkillEditorModule>("ActionSkillEditor").OnCloseEditorDelegate().Broadcast();
    return FWorkflowCentricApplication::OnRequestClose();
}

/**
 * Tick 
 * 目前看上去用不上, 暂时保留
 */
void FActionSkillEditor::Tick(float DeltaTime)
{
}

void FActionSkillEditor::CreatePreviewScene()
{
    if (!PreviewScene.IsValid())
    {
        PreviewScene = MakeShareable(
            new FActionSkillEditorPreviewScene(FPreviewScene::ConstructionValues()
                .AllowAudioPlayback(true)
                .ShouldSimulatePhysics(true)
                .SetEditor(true)
            )
        );
        PreviewScene->Initialize();
        PreviewScene->GetWorld()->GetWorldSettings()->SetIsTemporarilyHiddenInEditor(false);
    }
}

void FActionSkillEditor::ExtendToolbar()
{
    // 防止本方法本多次调用
    if (ToolbarExtender.IsValid())
    {
        RemoveToolbarExtender(ToolbarExtender);
        ToolbarExtender.Reset();
    }

    ToolbarExtender = MakeShareable(new FExtender);
    AddToolbarExtender(ToolbarExtender);

    ToolbarExtender->AddToolBarExtension(
        "Asset",
        EExtensionHook::After,
        GetToolkitCommands(),
        FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
        {
            // ToolbarBuilder.BeginSection("ActionSkill");
            // ToolbarBuilder.AddToolBarButton(
            // 	FUIAction(FExecuteAction::CreateSP(this, &FActionSkillEditor::NewAndEditSkill)),
            // 	NAME_None,
            // 	LOCTEXT("Create_ActionSkill_Label", "Create Skill"),
            // 	LOCTEXT("Create_ActionSkill_ToolTip", "Create a new skill"),
            // 	FSlateIcon(FEditorStyle::GetStyleSetName(), "Persona.CreateAsset")
            // );
            // ToolbarBuilder.EndSection();

            this->ExtendModeToolbar(ToolbarBuilder);
        }
    ));
}

void FActionSkillEditor::ExtendModeToolbar(FToolBarBuilder& InToolbarBuilder)
{
    AddToolbarWidget(SNew(SSpacer).Size(FVector2D(4.0f, 1.0f)));

    // Editor Mode
    AddToolbarWidget(
        SNew(SModeWidget, LOCTEXT("ActionSkill_EditorMode", "Edit Skill"), ActionSkillEditorModes::SkillEditorMode)
        .OnGetActiveMode(this, &FActionSkillEditor::GetCurrentMode)
        .OnSetActiveMode(this, &FActionSkillEditor::SetCurrentMode)
        .CanBeSelected(true)
        .ToolTipText(LOCTEXT("ActionSkill_EditMode_Tooltip", "Switch to Skill Editor Mode"))
        .IconImage(FEditorStyle::GetBrush("BTEditor.SwitchToBehaviorTreeMode"))
        .SmallIconImage(FEditorStyle::GetBrush("BTEditor.SwitchToBehaviorTreeMode.Small"))
    );

    // Add Separator
    AddToolbarWidget(
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .Padding(10.f)
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        .AutoWidth()
        [
            SNew(STextBlock)
            .Text(LOCTEXT("ActionSkillEditor_ModeSperator", "|"))
        ]
    );
        // SNew(STextBlock)
        // .Text("|")
        // // .BorderImage(FEditorStyle::GetBrush("BlueprintEditor.PipelineSeparator"))
        // .Padding(10.0f)

    // Define Mode
    AddToolbarWidget(
        SNew(SModeWidget, LOCTEXT("ActionSkill_DeclareMode", "Skill Struct"), ActionSkillEditorModes::SkillDeclareMode)
        .OnGetActiveMode(this, &FActionSkillEditor::GetCurrentMode)
        .OnSetActiveMode(this, &FActionSkillEditor::SetCurrentMode)
        .CanBeSelected(true)
        .ToolTipText(LOCTEXT("ActionSkill_DefineMode_Tooltip", "Switch to Skill Define Mode"))
        .IconImage(FEditorStyle::GetBrush("BTEditor.SwitchToBehaviorTreeMode"))
        .SmallIconImage(FEditorStyle::GetBrush("BTEditor.SwitchToBehaviorTreeMode.Small"))
    );

    AddToolbarWidget(SNew(SSpacer).Size(FVector2D(4.0f, 1.0f)));
}
// TSharedRef<IAssetFamily> FActionSkillEditor::CreateSkillAssetFamily()
// {
// 	return nullptr;
// }

TSharedRef<SDockTab> FActionSkillEditor::SpawnTab_Palette(const FSpawnTabArgs& Args)
{
    check(Args.GetTabId() == ActionSkillEditorTabs::SkillPalletteTab);
    
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
        .Icon(FEditorStyle::GetBrush("Kismet.Tabs.Palette"))
        .Label(LOCTEXT("ActionSkillPaletteTitle", "Palette"))
        [
            SNew( SBox )
            .AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ActionSkillPalette")))
            [
                Palette.ToSharedRef()
            ]
        ];

    return SpawnedTab;	
}

//~ Begin: IToolkit interface
void FActionSkillEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_ActionSkillEditor", "ActionSkill Editor"));
    auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    /** Register My Tabs */

/*	TabManager->RegisterTabSpawner( ActionSkillEditorTabs::PaletteTab, FOnSpawnTab::CreateSP(this, &FActionSkillEditor::SpawnTab_Palette) )
        .SetDisplayName( LOCTEXT("PaletteTab", "Palette") )
        .SetGroup( WorkspaceMenuCategoryRef )
        .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Tabs.Palette"))*/;
}

void FActionSkillEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
    InTabManager->UnregisterTabSpawner(ActionSkillEditorTabs::SkillPalletteTab);
	InTabManager->UnregisterTabSpawner(ActionSkillEditorTabs::EventPalletteTab);
}

FName FActionSkillEditor::GetToolkitFName() const
{
    return ACTIONSKILL_EDITOR_NAME;
}

FText FActionSkillEditor::GetBaseToolkitName() const
{
    return LOCTEXT("AppLabel", "ActionSkillEditor" );
}

FString FActionSkillEditor::GetWorldCentricTabPrefix() const
{
    return LOCTEXT("WorldCentricTabPrefix", "ActionSkillEditor" ).ToString();
}

FLinearColor FActionSkillEditor::GetWorldCentricTabColorScale() const
{
    return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}
//~ End: IToolkit interface

/**
 * 在此添加需要加入GC管理的成员变量
 */
void FActionSkillEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
}

//~ Begin: FEditorUndoClient interface
void FActionSkillEditor::PostUndo(bool bSuccess)
{
    OnPostUndo.Broadcast();
}

void FActionSkillEditor::PostRedo(bool bSuccess)
{
    OnPostUndo.Broadcast();
}
//~ End: FEditorUndoClient interface

//~ Begin: FTickableEditorObject interface
TStatId FActionSkillEditor::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(FActionSkillEditor, STATGROUP_Tickables);
}
//~ End: FTickableEditorObject interface

TSharedPtr<FActionSkillScope> FActionSkillEditor::NewAndEditSkill()
{
    auto SkillTable = FActionSkillModule::Get().GetSkillTable();
    TSharedPtr<FActionSkillScope> ActionSkill = SkillTable->NewActionSkill();
	SetEditingSkill(ActionSkill);
    OnNewSkillEvent.Broadcast(ActionSkill);
    return ActionSkill;
}

TSharedPtr<FActionEvent> FActionSkillEditor::NewAndEditEvent()
{
	auto EventTable = FActionSkillModule::Get().GetEventTable();
	TSharedPtr<FActionEvent> ActionEvent = EventTable->NewEvent();
	SetEditingEvent(ActionEvent);
	OnNewEventEvent.Broadcast(ActionEvent);
	return ActionEvent;
}

void FActionSkillEditor::NewEventStructureType()
{
    IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
    // TODO: Move this to project config
    const FString DefaultEventPath = "/Game/ActionSkill/Events";
    UObject* NewAsset = AssetTools.CreateAssetWithDialog("ASNewEvent", DefaultEventPath, UActionEventStructType::StaticClass(), NewObject<UActionEventStructTypeFactory>());
}

void FActionSkillEditor::DeleteAndEditSkill(TSharedPtr<FActionSkillScope> SkillToDel)
{
    if (SkillToDel.IsValid())
    {
        auto SkillTable = FActionSkillModule::Get().GetSkillTable();
        SkillTable->DeleteActionSkill(SkillToDel);
        OnDeleteSkillEvent.Broadcast(SkillToDel);
        SetEditingSkill(nullptr);
    }
}

void FActionSkillEditor::DeleteAndEditEvent(TSharedPtr<FActionEvent> EventToDel)
{
	if (EventToDel.IsValid())
	{
		int32 eventid = EventToDel.Get()->Id;
		auto SkillTable = FActionSkillModule::Get().GetSkillTable();
		SkillTable->DeleteActionEvent(eventid);
		auto EventTable = FActionSkillModule::Get().GetEventTable();
		EventTable->DeleteEvent(eventid);
		OnDeleteEventEvent.Broadcast(EventToDel);
		SetEditingEvent(nullptr);
	}
}

const UActionSkillTable* FActionSkillEditor::GetSkillTable() const
{
    return FActionSkillModule::Get().GetSkillTable();
}

const UActionEventTable* FActionSkillEditor::GetEventTable() const
{
	return FActionSkillModule::Get().GetEventTable();
}

void FActionSkillEditor::SetEditingSkill(TSharedPtr<FActionSkillScope> InActionSkill)
{
	//EditingActionEvent = nullptr;
	//EditingActionKeyFrame = nullptr;
	//EditingActionEventKeyFrame = nullptr;

    if (EditingActionSkill != InActionSkill)
    {
        EditingActionSkill = InActionSkill; 
        OnEditingSkillChangedEvent.Broadcast(EditingActionSkill);
    }
}

void FActionSkillEditor::SetEditingEvent(TSharedPtr<FActionEvent> InActionEvent)
{
	//EditingActionSkill = nullptr;
	EditingActionKeyFrame = nullptr;
	EditingActionEventKeyFrame = nullptr;

	if (EditingActionEvent != InActionEvent)
	{
		EditingActionEvent = InActionEvent;
		OnEditingEventChangedEvent.Broadcast(EditingActionEvent);
	}
}

void FActionSkillEditor::SetEditingActionKeyFrame(FActionKeyFrame* InActionKeyFrame)
{
	EditingActionEvent = nullptr;
	EditingActionEventKeyFrame = InActionKeyFrame != nullptr ? InActionKeyFrame->GetEvent() : nullptr;

    if (EditingActionKeyFrame != InActionKeyFrame)
    {
        EditingActionKeyFrame = InActionKeyFrame;
        if (OnEditingKeyFrameSetEvent.IsBound())
        {
            OnEditingKeyFrameSetEvent.Broadcast(EditingActionKeyFrame);
        }
    }
}

ISkillPreviewProxy* FActionSkillEditor::GetSkillPreviewProxy()
{
    IActionSkillEditorModule& ActionSkillModule = FModuleManager::GetModuleChecked<IActionSkillEditorModule>(TEXT("ActionSkillEditor"));
    return ActionSkillModule.GetSkillPreviewProxy(PreviewScene->GetWorld());
}

void FActionSkillEditor::LoadSkillTable()
{
    FActionSkillModule::Get().LoadSkillConfig();
    return;
}

void FActionSkillEditor::SaveAsset_Execute()
{
    FName CurrMode = GetCurrentMode();
    if (CurrMode == ActionSkillEditorModes::SkillEditorMode)
    {
        auto PickConfigPath = [] (const FString& Title){
            IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
            const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
            const FString DEFAULT_SAVE_PATH = FPaths::ProjectContentDir() + "Data/Maps/";
            FString CurrentFilename;

            const FString FileTypes = TEXT("Table CSV (*.csv)|*.csv");
            TArray<FString> OutFilenames;
            DesktopPlatform->SaveFileDialog(
                ParentWindowWindowHandle,
                Title,
                DEFAULT_SAVE_PATH,
                (CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetBaseFilename(CurrentFilename) + TEXT(".csv"),
                FileTypes,
                EFileDialogFlags::None,
                OutFilenames
                );
            if (OutFilenames.Num() > 0)
            {
                return OutFilenames[0];
            }

            return FString();
        };
        FString SkillConfigPath = GetDefault<UActionSkillSettings>()->SkillTablePath.FilePath;
        if (SkillConfigPath.IsEmpty())
        {
            SkillConfigPath = PickConfigPath(FString::Printf(TEXT("Export Skills as CSV... ")));
            GetMutableDefault<UActionSkillSettings>()->SkillTablePath.FilePath = SkillConfigPath;
        }

        FString EventConfigPath = GetDefault<UActionSkillSettings>()->EventTablePath.FilePath;
        if (EventConfigPath.IsEmpty())
        {
            EventConfigPath = PickConfigPath(FString::Printf(TEXT("Export Skill Events as CSV... ")));
            GetMutableDefault<UActionSkillSettings>()->EventTablePath.FilePath = EventConfigPath;
        }

        if (SkillConfigPath.IsEmpty() || EventConfigPath.IsEmpty())
        {
            UE_LOG(LogActionSkillEditor, Error, TEXT("Error to save skill data with empty path(%s, %s)!"), *SkillConfigPath, *EventConfigPath);
        }
        else
        {
            FString SkillText, EventText;
            if (FActionSkillModule::Get().ExportSkillConfigs(SkillText, EventText))
            {
                if (!FFileHelper::SaveStringToFile(SkillText, *SkillConfigPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
                {
                    UE_LOG(LogActionSkillEditor, Error, TEXT("Error to save skill data on path: %s"), *SkillConfigPath);
                }

                if (!FFileHelper::SaveStringToFile(EventText, *EventConfigPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
                {
                    UE_LOG(LogActionSkillEditor, Error, TEXT("Error to save skill data on path: %s"), *EventConfigPath);
                }
            }
            FActionSkillModule::Get().PostOnSaveData();
        }
    }
    else if (CurrMode == ActionSkillEditorModes::SkillDeclareMode)
    {
        TSharedPtr<FActionSkillDefineMode> DefineMode = StaticCastSharedPtr<FActionSkillDefineMode>(CurrentAppModePtr);
        if (DefineMode.IsValid())
        {
            TArray<UPackage*> PackagesToSave;
            for (UObject* ObjectToSave : DefineMode->GetObjectToSave())
            {
                if (ObjectToSave)
                {
                    PackagesToSave.AddUnique(ObjectToSave->GetOutermost());
                }
            }
            FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true/*bCheckDirtyOnAssetSave*/, /*bPromptToSave=*/ false);
        }
    }
}

void FActionSkillEditor::OnPreEditorModeDeactivated()
{
    OnEditingSkillChangedEvent.Clear();
    OnNewSkillEvent.Clear();
    OnDeleteSkillEvent.Clear();
	OnNewEventEvent.Clear();
	OnDeleteEventEvent.Clear();
}

class ACharacter* FActionSkillEditor::GetPreviewSkillCaster()
{
    return GetPrviewScene()->GetPrevCharacter();
}
#undef LOCTEXT_NAMESPACE
