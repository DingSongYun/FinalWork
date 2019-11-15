// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-02
#include "SSkillSegmentPanel.h"
#include "Engine/EngineTypes.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ClassViewer/Public/ClassViewerModule.h"
#include "ClassViewer/Public/ClassViewerFilter.h"
#include "TabFactory/SActionSkillOperatingTab.h"
#include "Components/SkeletalMeshComponent.h"
#include "PropertyCustomizationHelpers.h"
#include "Framework/Commands/Commands.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "SlateApplication.h"
#include "MessageDialog.h"
#include "ModuleManager.h"
#include "ActionSkillEditor.h"
#include "ActionSkill.h"
#include "ActionSkillEditorModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Serialization/SerializeUtils.h"
#include "Widgets/SActionEventPicker.h"
#include "EditorWidgetsModule.h"
#include "SScrubControlPanel.h"

#define LOCTEXT_NAMESPACE "SkillSegment"

/*********************************************************/
// FEventTrackCommands
/*********************************************************/
class FActionTrackCommands : public TCommands<FActionTrackCommands>
{
public:
    FActionTrackCommands()
        : TCommands<FActionTrackCommands>("ActionTrack", NSLOCTEXT("Contexts", "ActionTrack", "Action Track"), NAME_None, FEditorStyle::GetStyleSetName())
    {

    }

    TSharedPtr<FUICommandInfo> DeleteNotify;

    virtual void RegisterCommands() override
    {
        UI_COMMAND(DeleteNotify, "Delete", "Deletes the selected node.", EUserInterfaceActionType::Button, FInputChord(EKeys::Platform_Delete));
    }
};

/*********************************************************/
// SActionTrack
/*********************************************************/
void SActionTrack::Construct(const FArguments& InArgs, FAction& InAction)
{
    MyAction = &InAction;

    this->ChildSlot
    [
        SAssignNew(TrackWidget, STrackWidget)
        .ViewInputMin(InArgs._ViewInputMin)
        .ViewInputMax(InArgs._ViewInputMax)
        .CursorPosition(InArgs._CursorPosition)
        .TrackColor(GetTrackColor())
        .OnBuildContextMenu(this, &SActionTrack::BuildContextMenu)
        .OnTrackDragDrop(this, &SActionTrack::OnTrackDragDrop)
    ];

    ConstructNodes(TrackWidget, MyAction);
    BindCommands();
}

void SActionTrack::ReconstructNodes()
{
   TrackWidget->ClearTrack();
   ConstructNodes(TrackWidget, MyAction);
}

FReply SActionTrack::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if(UICommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SActionTrack::BindCommands()
{
    if (UICommandList.IsValid()) return ;
    UICommandList = MakeShareable(new FUICommandList);
    FActionTrackCommands::Register();
    BindCommands(FActionTrackCommands::Get());
}

void SActionTrack::BindCommands(const FActionTrackCommands& ActionCommands)
{
    // Implemtation by sub-class
}

/*********************************************************/
// SAnimTrack
/*********************************************************/
#pragma region SAnimTrack
void SAnimTrack::Construct(const FArguments& InArgs, FAction& InAction, FOnAssignAnim& OnAssignAnimDelegate)
{
    SActionTrack::Construct(InArgs, InAction);

    OnAssignAnim = OnAssignAnimDelegate;
}

void SAnimTrack::ConstructNodes(TSharedPtr<STrackWidget>& Track, const FAction* InAction)
{
    if (!Track.IsValid()) return ;
    if (InAction->AnimReference.IsNull()) return ;

    static const FLinearColor NodeColor = FLinearColor(0.f, 0.5f, 0.f, 0.5f);
    Track->AddTrackNode(
        SNew(STrackNodeWidget)
        .NodeName(InAction->AnimName)
        .NodeColor(NodeColor)
        .NodeStartPos(0)
        .NodeBlockLength(-1)
        .NodeBlockHeight(-1)
        .AllowDrag(false)
    );
}

bool SAnimTrack::BuildContextMenu(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.BeginSection("Anim", LOCTEXT("SkillAnim", "SkillAnim"));
    {
        MenuBuilder.AddMenuEntry(
            LOCTEXT("DelSkillAnim_Lable", "Remove"),
            LOCTEXT("DelSkillAnim_ToolTip", "Remove Animation"),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateRaw(this, &SAnimTrack::ClearAnimation)),
            NAME_None,
            EUserInterfaceActionType::Button
        );
    }
    MenuBuilder.EndSection();
    return true;
}

FLinearColor SAnimTrack::GetTrackColor()
{
    return FLinearColor::White;
}

void SAnimTrack::ClearAnimation()
{
    if (OnAssignAnim.IsBound())
    {
        OnAssignAnim.Execute(nullptr);
    }
}

void SAnimTrack::OnTrackDragDrop(TSharedPtr<FDragDropOperation> DragDropOp, float DataPos)
{
    if (DragDropOp.IsValid() && DragDropOp->IsOfType<FAssetDragDropOp>())
    {
        TSharedPtr<FAssetDragDropOp> AssetOp = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOp);
        if (AssetOp->HasAssets())
        {
            if (UAnimSequenceBase* DroppedSequence = FAssetData::GetFirstAsset<UAnimSequenceBase>(AssetOp->GetAssets()))
            {
                if (OnAssignAnim.IsBound())
                {
                    OnAssignAnim.Execute(DroppedSequence);
                }
            }
        }
    }
}
#pragma endregion SAnimTrack

/*********************************************************/
// SEventTack
/*********************************************************/
#pragma region SEventTrack

SEventTrack::SEventTrack()
    : SActionTrack(), ActionSkillEditorPtr(nullptr), CurrSelectedKey(nullptr)
{

}

SEventTrack::~SEventTrack()
{
	if(ActionSkillEditorPtr.IsValid() && DeleteEventHandle.IsValid())
		ActionSkillEditorPtr.Pin()->OnDeleteEvent().Remove(DeleteEventHandle);
	if (ActionSkillEditorPtr.IsValid() && EditingEventChangedHandle.IsValid())
		ActionSkillEditorPtr.Pin()->OnEditingEventChanged().Remove(EditingEventChangedHandle);
}

void SEventTrack::Construct(const FArguments& InArgs, FAction& InAction, TWeakPtr<class FActionSkillEditor> InActionSkillEditorPtr)
{
    SActionTrack::Construct(InArgs, InAction);

	ActionSkillEditorPtr = InActionSkillEditorPtr;
	check(ActionSkillEditorPtr.IsValid());
	DeleteEventHandle = ActionSkillEditorPtr.Pin()->OnDeleteEvent().AddRaw(this, &SEventTrack::OnDelEvent);
	EditingEventChangedHandle = ActionSkillEditorPtr.Pin()->OnEditingEventChanged().AddSP(this, &SEventTrack::OnEditingEventChanged);
}

FLinearColor SEventTrack::GetTrackColor()
{
    return FLinearColor::White;
}

FReply SEventTrack::OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
    DeselectKeys();
    return FReply::Unhandled();
}

bool SEventTrack::OnBuildNodeContextMenu(FMenuBuilder& MenuBuilder, FActionKeyFrame* KeyFrame)
{
    MenuBuilder.PushCommandList(UICommandList.ToSharedRef());
    MenuBuilder.BeginSection("KeyFrame", LOCTEXT("KeyFrame", "KeyFrame"));
    {
        MenuBuilder.AddMenuEntry(
            LOCTEXT("KeyFrame_CopyNotify_Lable", "Copy Notify ..."),
            LOCTEXT("KeyFrame_CopyNotify_ToolTip", "Copy Notify"),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateRaw(this, &SEventTrack::OnCopyKeyFrameToClipboard, KeyFrame))
        );
		MenuBuilder.AddMenuEntry(
			LOCTEXT("KeyFrame_DeleteNotify_Lable", "Delete Notify ..."),
			LOCTEXT("KeyFrame_DeleteNotify_ToolTip", "Delete Notify"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &SEventTrack::DeleteNotifyByFrame, KeyFrame))
		);
        // Disable SubEvent
        /*
        MenuBuilder.AddSubMenu(
            LOCTEXT("KeyFrame_AddSubNotify_Lable", "Add SubNotify..."),
            LOCTEXT("KeyFrame_AddSubNotify_ToolTip", "Add Sub ActionNotify"),
            FNewMenuDelegate::CreateRaw( this, &SEventTrack::FillNewSubEventMenu, KeyFrame) 
        );
        */
    }
    MenuBuilder.EndSection();
    return true;
}

bool SEventTrack::BuildContextMenu(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.BeginSection("Event", LOCTEXT("SkillEvent", "SkillEvent"));
    {
        MenuBuilder.AddSubMenu(
            LOCTEXT("SkillEvent_Lable", "Add Event..."),
            LOCTEXT("SkillEvent_ToolTip", "Add Action Event"),
            FNewMenuDelegate::CreateRaw( this, &SEventTrack::FillNewEventMenu) );
        MenuBuilder.AddMenuEntry(
            LOCTEXT("KeyFrame_PasterNotify_Lable", "Paster Notify ..."),
            LOCTEXT("KeyFrame_PasterNotify_ToolTip", "Paster Notify"),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateRaw(this, &SEventTrack::OnPasterKeyFrame))
        );
    }
    MenuBuilder.EndSection();
    return true;
}

void SEventTrack::BindCommands(const FActionTrackCommands& Commands)
{
    UICommandList->MapAction(
        Commands.DeleteNotify,
        FExecuteAction::CreateSP(this, &SEventTrack::OnDeletePressed));
}

void SEventTrack::OnDeletePressed()
{
    if (HasKeyboardFocus() || HasFocusedDescendants())
    {
        DeleteSelectedNodes();
    }
}

void SEventTrack::DeleteNotifyByFrame(FActionKeyFrame * KeyFrame)
{
	DeleteEvent(KeyFrame);
	CurrSelectedKey = nullptr;
	ActionSkillEditorPtr.Pin()->SetEditingActionKeyFrame(nullptr);
}

void SEventTrack::DeleteSelectedNodes()
{
    TArray<const FActionKeyFrame*> SelectedNodes;
    for (auto Pair : KeyNodes)
    {
        if (Pair.Value->IsSelected())
        {
            SelectedNodes.Add(Pair.Key);
        }
    }

    for (const FActionKeyFrame* KeyToDel : SelectedNodes)
    {
        DeleteEvent(KeyToDel);
    }

    CurrSelectedKey = nullptr;
    ActionSkillEditorPtr.Pin()->SetEditingActionKeyFrame(nullptr);
}

void SEventTrack::FillNewEventMenu(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.AddSubMenu(
        LOCTEXT("SkillEvent_Lable", "New Event..."),
        LOCTEXT("SkillEvent_ToolTip", "New Action Event"),
        FNewMenuDelegate::CreateLambda( [&](FMenuBuilder& InSubMenuBuilder)
        {
            CreateEventStructPicker(InSubMenuBuilder, [&](const FAssetData& AssetData) {
                OnActionEventAssetSelected(AssetData);
            });
        })
    );

    MenuBuilder.BeginSection("ActionEventSubMenu", LOCTEXT("SkillEventSubMenu_Section", "Action Events"));
    MenuBuilder.AddWidget(
        SNew(SActionEventPicker)
        .OnSelectEvent_Lambda([this](TSharedPtr<FActionEventEntry> SelectedEvent, ESelectInfo::Type SelectInfo){
            this->AddEventToAction(SelectedEvent->EventPtr);
			FSlateApplication::Get().DismissAllMenus();
        }),
        FText(), true, false);
    MenuBuilder.EndSection();
}

void SEventTrack::FillNewSubEventMenu(FMenuBuilder& MenuBuilder, FActionKeyFrame* KeyFrame)
{
	MenuBuilder.AddSubMenu(
		LOCTEXT("SkillEvent_Lable", "New Event..."),
		LOCTEXT("SkillEvent_ToolTip", "New Action Event"),
		FNewMenuDelegate::CreateLambda([&, KeyFrame](FMenuBuilder& InSubMenuBuilder)
		{
			CreateEventStructPicker(InSubMenuBuilder, [&, KeyFrame](const FAssetData& AssetData)
			{
				if (UObject* NotifyStruct = AssetData.GetAsset())
				{
					FActionEventPtr NewEvent = FActionSkillModule::Get().GetEventTable()->NewEvent();
					AddNewSubEvent(Cast<UScriptStruct>(NotifyStruct), KeyFrame, NewEvent);
				}
				FSlateApplication::Get().DismissAllMenus();
			});
		})
	);

	MenuBuilder.BeginSection("ActionEventSubMenu", LOCTEXT("SkillEventSubMenu_Section", "Action Events"));
	MenuBuilder.AddWidget(
		SNew(SActionEventPicker)
		.OnSelectEvent_Lambda([this, KeyFrame](TSharedPtr<FActionEventEntry> SelectedEvent, ESelectInfo::Type SelectInfo)
		{
			AddNewSubEvent(SelectedEvent->EventPtr->StaticStruct(), KeyFrame, SelectedEvent->EventPtr);
			FSlateApplication::Get().DismissAllMenus();
		}),
		FText(), true, false
	);
	MenuBuilder.EndSection();
}

template<typename FunctorType> void SEventTrack::CreateEventStructPicker(FMenuBuilder& MenuBuilder, FunctorType&& OnPickEventStructFunc)
{
#if 0
    class FNotifyClassFilter : public IClassViewerFilter
    {
    public:
        bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< class FClassViewerFilterFuncs > InFilterFuncs ) override
        {
            return InClass->IsChildOf(UActionNotify::StaticClass())
                && !InClass->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);
        }

        bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const class IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< class FClassViewerFilterFuncs > InFilterFuncs) override
        {
            return InUnloadedClassData->IsChildOf(UActionNotify::StaticClass())
                && !InUnloadedClassData->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);
        }

    };
    FClassViewerInitializationOptions InitOptions;
    InitOptions.Mode = EClassViewerMode::ClassPicker;
    InitOptions.bShowObjectRootClass = false;
    InitOptions.bShowUnloadedBlueprints = true;
    InitOptions.bShowNoneOption = false;
    InitOptions.bEnableClassDynamicLoading = true;
    InitOptions.bExpandRootNodes = true;
    InitOptions.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
    InitOptions.ClassFilter = MakeShared<FNotifyClassFilter>();
    InitOptions.bShowBackgroundBorder = false;

    FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
    MenuBuilder.AddWidget(
        SNew(SBox)
        .MinDesiredWidth(300.0f)
        .MaxDesiredHeight(400.0f)
        [
            ClassViewerModule.CreateClassViewer(InitOptions,
                FOnClassPicked::CreateLambda([this](UClass* InClass)
                {
                    FSlateApplication::Get().DismissAllMenus();
                    AddNewNotify(InClass->GetName(), InClass);
                }
            ))
        ],
        FText(), true, false);
#else 
    FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
    FAssetPickerConfig AssetPickerConfig;
    {
        AssetPickerConfig.Filter.ClassNames.Add(UScriptStruct::StaticClass()->GetFName());
        AssetPickerConfig.Filter.bRecursiveClasses = true;
        // AssetPickerConfig.Filter.PackagePaths.Add("/Game/ActionSkill");
        AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
        AssetPickerConfig.bAllowNullSelection = false;
        AssetPickerConfig.bFocusSearchBoxWhenOpened = true;
        AssetPickerConfig.bAllowDragging = false;
        AssetPickerConfig.SaveSettingsName = TEXT("AssetPropertyPicker");
        // AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SEventTrack::OnActionEventAssetSelected);
        AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda(OnPickEventStructFunc);
        AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SEventTrack::FilterActionEventStruct);
    }
    MenuBuilder.AddWidget(
        SNew(SBox)
        .MinDesiredWidth(300.0f)
        .MaxDesiredHeight(400.0f)
        [
            ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
        ],
        FText(), true, false);
#endif
}

//TODO: Remove
void SEventTrack::OnActionEventAssetSelected(const FAssetData& AssetData)
{
    UE_LOG(LogActionSkillEditor, Warning, TEXT("Pick action notify asset!!!"));

    if (UObject* NotifyStruct = AssetData.GetAsset())
    {
        AddNewEvent(NotifyStruct->GetName(), NotifyStruct);
    }

    // Close Event Struct Picker
    FSlateApplication::Get().DismissAllMenus();
}

bool SEventTrack::FilterActionEventStruct(const FAssetData& AssetData)
{
    if (UClass* Cls = AssetData.GetClass())
    {
        #if 0 // USE_METADATA
            UPackage* Package = Cls->GetOuterUPackage();
            bool HasMetaData = Cls->HasMetaDataHierarchical(TEXT("ActionSkillNotify"));
            UE_LOG(LogTemp, Warning, TEXT("has actionskill metadata: %d, %s"), HasMetaData, *Package->GetName());
        #else
            bool IsActionEventStruct = Cls == UActionEventStructType::StaticClass();
            return !IsActionEventStruct;
        #endif
    }

    return true;
}

void SEventTrack::AddEventToAction(FActionEventPtr InEvent)
{
    AddEventToAction(InEvent, TrackWidget->LastClickTime);
}

void SEventTrack::AddEventToAction(FActionEventPtr InEvent, float StartTime)
{
    if (!InEvent.IsValid()) return ;

    FActionKeyFrame& ActionKeyFrame = MyAction->Keys.AddDefaulted_GetRef();
    ActionKeyFrame.Time = StartTime;
    ActionKeyFrame.Name = InEvent->Name;
    ActionKeyFrame.EventId = InEvent->Id;

    ReconstructNodes();
}

void SEventTrack::AddNewEvent(FString NewNotifyName, UObject* NotifyStruct)
{
    AddNewEvent(NewNotifyName, Cast<UScriptStruct>(NotifyStruct), TrackWidget->LastClickTime);
}

void SEventTrack::AddNewEvent(FString NewNotifyName, UScriptStruct* NotifyStruct, float StartTime)
{
    if (!NotifyStruct) return ;
    FActionEventPtr NewEvent = ActionSkillEditorPtr.Pin().Get()->NewAndEditEvent();
    NewEvent->Name = NotifyStruct->GetFName();
    NewEvent->Params.Initialize(*NotifyStruct);
    NewEvent->Params.FixedUserStructType();
	NewEvent->MarkDirty(true);
    AddEventToAction(NewEvent, StartTime);
}

void SEventTrack::AddNewEvent(const FActionKeyFrame& InKeyFrame, float StartTime)
{
	int Index = MyAction->Keys.Add(InKeyFrame);
	MyAction->Keys[Index].Time = StartTime;

    ReconstructNodes();
}

void SEventTrack::AddNewSubEvent(UScriptStruct * NotifyStruct, FActionKeyFrame * KeyFrame, FActionEventPtr NewEvent)
{
	if (!NotifyStruct) return ;

    FActionEventPtr KeyEvent = KeyFrame->GetEvent();
    FUserStructOnScope& NewSubEvent = KeyEvent->SubEvents.AddDefaulted_GetRef();
    NewSubEvent.Initialize(UActionSkillTable::GetSubEventStruct());
    NewSubEvent.SetStructPropertyValue("EventId", NewEvent->Id);

    if (UProperty* Prop = NewSubEvent.GetStructPropertyByName("EventArgs"))
    {
        if (FUserStructOnScope* Args = Prop->ContainerPtrToValuePtr<FUserStructOnScope>(&NewSubEvent, 0))
        {
            Args->SetUserStructBaseType(UActionEventArgsStruct::StaticClass());
        }
    }
    NewSubEvent.FixedUserStructType();
}

void SEventTrack::DeleteEvent(const FActionKeyFrame* KeyToDel)
{
    if (!KeyToDel) return ;
    int32 Index = MyAction->Keys.IndexOfByPredicate([&](const FActionKeyFrame& InKeyFrame) { return &InKeyFrame == KeyToDel; });
    if (Index != INDEX_NONE)
    {
        MyAction->Keys.RemoveAt(Index);
    }
    ReconstructNodes();
}

void SEventTrack::OnCopyKeyFrameToClipboard(FActionKeyFrame* KeyFrame)
{
    FString CopyString;
    if (StructSerializer::ExportString(CopyString, FActionKeyFrame::StaticStruct(), KeyFrame, PPF_None))
    {
        FPlatformApplicationMisc::ClipboardCopy(*CopyString);
    }
}

void SEventTrack::OnPasterKeyFrame()
{
    FString PasterString;
    FPlatformApplicationMisc::ClipboardPaste(PasterString);

    FSerializeResult ImportResult;
    FActionKeyFrame NewKeyFrame;
    UScriptStruct* Struct = FActionKeyFrame::StaticStruct();
    if (StructSerializer::ImportString(PasterString, Struct , &NewKeyFrame, PPF_None, Struct->GetName(), ImportResult))
    {
        AddNewEvent(NewKeyFrame, TrackWidget->LastClickTime);
    }
}

void SEventTrack::OnDelEvent(TWeakPtr<struct FActionEvent> EventObj)
{
	if (!EventObj.IsValid() || MyAction == nullptr) return;
	int32 Index = MyAction->Keys.IndexOfByPredicate([&](const FActionKeyFrame& InKeyFrame) { return InKeyFrame.EventId == EventObj.Pin()->Id; });
	if (Index != INDEX_NONE)
	{
		MyAction->Keys.RemoveAt(Index);
	}
	ReconstructNodes();
}

void SEventTrack::OnEditingEventChanged(TWeakPtr<struct FActionEvent> EventObj)
{
	if (EventObj.IsValid()) 
	{
		if (CurrSelectedKey != nullptr)
		{
			const FActionKeyFrame* key = CurrSelectedKey;
			CurrSelectedKey = nullptr;
			KeyNodes[key]->Deselect();
		}
	}
}

void SEventTrack::ConstructNodes(TSharedPtr<STrackWidget>& Track, const FAction* InAction)
{
    check(Track.IsValid() && InAction);
    CurrSelectedKey = nullptr;
    KeyNodes.Empty();
    SortActionKyes();
    static const FLinearColor NodeColor = FLinearColor(0.5f, 0.0f, 0.f, 0.3f);
    for(const FActionKeyFrame& KeyFrame : InAction->Keys)
    {
        TSharedPtr<STrackNodeWidget> TrackNode;
        TrackWidget->AddTrackNode(
            SAssignNew(TrackNode, STrackNodeWidget)
            .NodeColor(NodeColor)
            .NodeBlockLength(0)
            .NodeName(this, &SEventTrack::GetNodeName, &KeyFrame)
            .NodeStartPos(this, &SEventTrack::GetNodeStartPos, &KeyFrame)
            .OnSelectionChanged(this, &SEventTrack::OnActionKeySelectionChanged, &KeyFrame)
            .OnDataPositionChanged(this, &SEventTrack::ChangeActionKeyTime, const_cast<FActionKeyFrame*>(&KeyFrame))
            .OnBuildNodeContextMenu(this, &SEventTrack::OnBuildNodeContextMenu, const_cast<FActionKeyFrame*>(&KeyFrame))
        );

        KeyNodes.Emplace(&KeyFrame, TrackNode);
    }
}

FName SEventTrack::GetNodeName(const FActionKeyFrame* KeyFrame) const
{
    return KeyFrame->Name;
}

float SEventTrack::GetNodeStartPos(const FActionKeyFrame* KeyFrame) const
{
    return KeyFrame->Time;
}

void SEventTrack::OnActionKeySelectionChanged(bool bIsSelected, const FActionKeyFrame* KeyFrame)
{
    if (!KeyFrame) return;

    bool bNotifyKeySelectionChanged = false;
    if (CurrSelectedKey != nullptr)
    {
        if (bIsSelected)
        {
            if (KeyFrame != CurrSelectedKey)
            {
                KeyNodes[CurrSelectedKey]->Deselect();
                CurrSelectedKey = KeyFrame;
                bNotifyKeySelectionChanged = true;
            }
        }
        else
        {
            if (KeyFrame == CurrSelectedKey)
            {
                CurrSelectedKey = nullptr;
                bNotifyKeySelectionChanged = true;
            }
        }
    }
    else if (bIsSelected)
    {
        CurrSelectedKey = KeyFrame;
        bNotifyKeySelectionChanged = true;
    }

    if (bNotifyKeySelectionChanged)
    {
        ActionSkillEditorPtr.Pin()->SetEditingActionKeyFrame(const_cast<FActionKeyFrame*>(KeyFrame));
    }
}

void SEventTrack::ChangeActionKeyTime(float NewKeyTime, FActionKeyFrame* KeyFrame)
{
    KeyFrame->Time = NewKeyTime;
    SortActionKyes();
}

void SEventTrack::SortActionKyes()
{
    struct FSortKeyTime
    {
        bool operator()( const FActionKeyFrame &A, const FActionKeyFrame &B ) const
        {
            return A.Time < B.Time;
        }
    };
    static const FSortKeyTime SortKeyTime;
    MyAction->Keys.Sort(SortKeyTime);
}

void SEventTrack::DeselectKeys()
{
    if (CurrSelectedKey && KeyNodes.Contains(CurrSelectedKey))
    {
        KeyNodes[CurrSelectedKey]->Deselect();
    }
}
#pragma endregion SEventTrack

struct FActionSkillSingleActionControl
{
public:
	static FReply OnForwardPlayCilck(SSkillSegmentPanel* Operator)
	{
		bool bIsPlaying = Operator->IsPlayingActionSkillSingleAction();
		if (bIsPlaying)
		{
			Operator->PauseActionSkillSingleAction();
		}
		else
		{
			Operator->PlayActionSkillSingleAction();
		}
		return FReply::Handled();
	}

	static EPlaybackMode::Type GetPlaybackMode(SSkillSegmentPanel* Operator)
	{
		bool bIsPlaying = Operator->IsPlayingActionSkillSingleAction();
		return bIsPlaying ? EPlaybackMode::PlayingForward : EPlaybackMode::Stopped;
	}
};

/*********************************************************/
// SSkillSegmentPanel
/*********************************************************/
void SSkillSegmentPanel::Construct(const FArguments& InArgs, TWeakPtr<class SActionSkillOperatingTab> HostEditor, FAction& InAction)
{
	check(HostEditor.IsValid())
    OptEditorPtr 			= HostEditor;
    ActionPtr 				= &InAction;
	ActionIndex             = InArgs._ActionIndexInSkill;
	ActionStartPos          = InArgs._ActionStartPosInSkill;
    OnPreActionUpdate		= InArgs._OnPreActionUpdate;
    OnPostActionUpdate		= InArgs._OnPostActionUpdate;
    OnSkillActionClicked	= InArgs._OnSkillActionClicked;
	FEditorWidgetsModule& EditorWidgetsModule = FModuleManager::Get().LoadModuleChecked<FEditorWidgetsModule>("EditorWidgets");
	FTransportControlArgs TransportControlArgs;
	{
		TransportControlArgs.OnForwardPlay = FOnClicked::CreateStatic(&FActionSkillSingleActionControl::OnForwardPlayCilck, this);
		TransportControlArgs.OnGetPlaybackMode = FOnGetPlaybackMode::CreateStatic(&FActionSkillSingleActionControl::GetPlaybackMode, this);
	}

	this->ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		[
			SAssignNew(ScrubPanel, SSkillSingleActionScrubPanel, this)
			.ViewInputMin(this, &SSkillSegmentPanel::GetViewMinInput)
			.ViewInputMax(this, &SSkillSegmentPanel::GetViewMaxInput)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
        [
            SNew(SExpandableArea)
            .HeaderContent()
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Left)
                .FillWidth(1)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT( "ActionLabel" , "Action"))
                    .ShadowOffset(FVector2D(1.0f, 1.0f))
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Right)
                [
                    SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(0.5f)
						[
							EditorWidgetsModule.CreateTransportControl(TransportControlArgs)
						]
					]
                    
                    + SHorizontalBox::Slot()
                    .AutoWidth()
					.VAlign(VAlign_Center)
                    [
                        PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &SSkillSegmentPanel::NewAction))
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
					.VAlign(VAlign_Center)
                    [
                        PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &SSkillSegmentPanel::DeleteAction))
                    ]
                ]
            ]
            .BodyContent()
            [
                SAssignNew(PanelArea, SBorder)
                .BorderImage( FEditorStyle::GetBrush("NoBorder") )
                .Padding(0.f)
                .ColorAndOpacity( FLinearColor::White )
            ]
        ]
    ];

    ConstructAction();
}

void SSkillSegmentPanel::ConstructAction()
{
    TSharedPtr<SVerticalBox> TrackPanel;
    PanelArea->SetContent(
        SAssignNew( TrackPanel, SVerticalBox )
    );

    // Anim Track
    SAnimTrack::FOnAssignAnim OnAssignAnimDelegate;
    OnAssignAnimDelegate.BindRaw(this, &SSkillSegmentPanel::AssignAnimSegment);

    TrackPanel->AddSlot()
    .AutoHeight()
    .VAlign(VAlign_Center)
    .Padding(0.0f)
    [
        SNew(SAnimTrack, *ActionPtr, OnAssignAnimDelegate)
		.ViewInputMin(this, &SSkillSegmentPanel::GetViewMinInput)
		.ViewInputMax(this, &SSkillSegmentPanel::GetViewMaxInput)
        .CursorPosition(this, &SSkillSegmentPanel::GetScrubValue)
    ];

    // Event Track
    TrackPanel->AddSlot()
    .AutoHeight()
    .VAlign(VAlign_Center)
    .Padding(FMargin(0, 5.f, 0, 0))
    [
        SNew(SEventTrack, *ActionPtr, OptEditorPtr.Pin()->GetActionSkillEditorPtr())
        .ViewInputMin(this, &SSkillSegmentPanel::GetViewMinInput)
        .ViewInputMax(this, &SSkillSegmentPanel::GetViewMaxInput)
        .CursorPosition(this, &SSkillSegmentPanel::GetScrubValue)
    ];

	if (ActionIndex.Get() == 0 && ActionPtr->AnimName.IsValid() && !ActionPtr->AnimName.IsNone())
	{
		OptEditorPtr.Pin()->ForceSetActionSkillPosition(0);
	}
}

void SSkillSegmentPanel::Reconstruct()
{
    ConstructAction();
}

FReply SSkillSegmentPanel::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    bool bLeftMouseButton = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
    if (bLeftMouseButton)
    {
        this->ProcessClickSegment();
        return FReply::Handled();
    }

    return FReply::Unhandled();
}

void SSkillSegmentPanel::ProcessClickSegment()
{
    if (OnSkillActionClicked.IsBound())
    {
        OnSkillActionClicked.Execute(*ActionPtr);
    }
}

void SSkillSegmentPanel::PlayActionSkillSingleAction()
{
	OptEditorPtr.Pin()->PlayActionSkillSingleAction(*ActionPtr, ActionStartPos.Get());
}

bool SSkillSegmentPanel::IsPlayingActionSkillSingleAction()
{
	return OptEditorPtr.Pin()->IsPlayingActionSkillSingleAction(*ActionPtr);
}

void SSkillSegmentPanel::PauseActionSkillSingleAction()
{
	OptEditorPtr.Pin()->PauseActionSkillSingleAction();
}

class UActionSkillComponent * SSkillSegmentPanel::GetSkillComponent() const
{
	return OptEditorPtr.Pin()->GetSkillComponent();
}

float SSkillSegmentPanel::GetViewMinInput() const
{
	return 0.0f;
}

float SSkillSegmentPanel::GetViewMaxInput() const
{
	return ActionPtr->GetActionLength();
}

float SSkillSegmentPanel::GetScrubValue() const
{
	if (auto SkillComp = GetSkillComponent())
	{
		float total = SkillComp->GetPosition();
		float length = total - ActionStartPos.Get();
		if (length > GetViewMinInput() && length <= GetViewMaxInput())
		{
			return length;
		}
	}
	return 0;
}

class FActionUpdateScope
{
public:
    FActionUpdateScope(FAction& InAction, FOnPreActionUpdate& PreUpdate, FOnPostActionUpdate& PostUpdate)
        : ActionRef(InAction), OnPreUpdate(PreUpdate), OnPostUpdate(PostUpdate)
    {
        if (OnPreUpdate.IsBound())
        {
            OnPreUpdate.Execute(ActionRef);
        }
    }

    ~FActionUpdateScope()
    {
        if (OnPostUpdate.IsBound())
        {
            OnPostUpdate.Execute(ActionRef);
        }
    }

private:
    FAction&				ActionRef;
    FOnPreActionUpdate&		OnPreUpdate;
    FOnPostActionUpdate&	OnPostUpdate;
};

void SSkillSegmentPanel::AssignAnimSegment(UAnimSequenceBase* NewAnim)
{
    // if (NewAnim == nullptr) return;
    FActionUpdateScope UpdateScope(*ActionPtr, OnPreActionUpdate, OnPostActionUpdate);

    ActionPtr->AnimName = NewAnim ? *NewAnim->GetName() : FName();
    ActionPtr->AnimReference = NewAnim;

    if (NewAnim)
    {
        TWeakPtr<class FActionSkillEditor> ActionSkillEditorPtr = OptEditorPtr.Pin()->GetActionSkillEditorPtr();
        if (ACharacter* Caster = ActionSkillEditorPtr.Pin()->GetPreviewSkillCaster())
        {
            UAnimInstance* AnimInstance = Caster->GetMesh()->GetAnimInstance();
            if (AnimInstance && !NewAnim->GetSkeleton()->IsCompatible(AnimInstance->CurrentSkeleton))
            {
                FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(
                    FString::Printf(TEXT("Animation is not compatible with current preview character!!!"))
                ));
            }
        }
    }

    Reconstruct();
}

void SSkillSegmentPanel::NewAction()
{
    OptEditorPtr.Pin()->AddAction();
}

void SSkillSegmentPanel::DeleteAction()
{
    OptEditorPtr.Pin()->DeleteAction(*ActionPtr);
}

/*********************************************************/
// SSkillSingleActionScrubPanel
/*********************************************************/
void SSkillSingleActionScrubPanel::Construct(const FArguments& InArgs, SSkillSegmentPanel* InSegmentPanel)
{
	SegmentPanel = InSegmentPanel;
	ViewMinInput = InArgs._ViewInputMin;
	ViewMaxInput = InArgs._ViewInputMax;
	this->ChildSlot
	[
		SNew(SHorizontalBox)
		.AddMetaData<FTagMetaData>(TEXT("AnimScrub.Scrub"))
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.FillWidth(1)
		.Padding(0.0f)
		[
			SAssignNew(ScrubControlPanel, SScrubControlPanel)
			.IsEnabled(true)
			.Value(this, &SSkillSingleActionScrubPanel::GetScrubValue)
			.NumOfKeys(this, &SSkillSingleActionScrubPanel::GetNumOfFrames)
			.SequenceLength(this, &SSkillSingleActionScrubPanel::GetSequenceLength)
			.OnValueChanged(this, &SSkillSingleActionScrubPanel::OnScrubValueChanged)
			.ViewInputMin(InArgs._ViewInputMin)
			.ViewInputMax(InArgs._ViewInputMax)
			.DisplayDrag(true)
			.bAllowZoom(true)
			.IsRealtimeStreamingMode(this, &SSkillSingleActionScrubPanel::IsRealtimeStreamingMode)
		]
	];
}

float SSkillSingleActionScrubPanel::GetScrubValue() const
{
	return SegmentPanel->GetScrubValue();
}

uint32 SSkillSingleActionScrubPanel::GetNumOfFrames() const
{
	return SegmentPanel->ActionPtr->GetActionFrame();
}

float SSkillSingleActionScrubPanel::GetSequenceLength() const
{
	return SegmentPanel->ActionPtr->GetActionLength();
}

bool SSkillSingleActionScrubPanel::IsRealtimeStreamingMode() const
{
	return false;
}

void SSkillSingleActionScrubPanel::OnScrubValueChanged(float NewValue)
{
	if(NewValue >= ViewMinInput.Get() && NewValue <= ViewMaxInput.Get())
		SegmentPanel->OptEditorPtr.Pin()->SampleActionSkillAtPosition(SegmentPanel->ActionStartPos.Get() + NewValue);
}

FReply SSkillSingleActionScrubPanel::OnClick_Forward_Step()
{
	return FReply::Handled();
}

FReply SSkillSingleActionScrubPanel::OnClick_Forward_End()
{
	return FReply::Handled();
}

FReply SSkillSingleActionScrubPanel::OnClick_Backward_Step()
{
	return FReply::Handled();
}

FReply SSkillSingleActionScrubPanel::OnClick_Backward_End()
{
	return FReply::Handled();
}

FReply SSkillSingleActionScrubPanel::OnClick_Forward()
{
	return FReply::Handled();
}

FReply SSkillSingleActionScrubPanel::OnClick_Backward()
{
	return FReply::Handled();
}

FReply SSkillSingleActionScrubPanel::OnClick_ToggleLoop()
{
	return FReply::Handled();
}

FReply SSkillSingleActionScrubPanel::OnClick_Record()
{
	return FReply::Handled();
}

EPlaybackMode::Type SSkillSingleActionScrubPanel::GetPlaybackMode() const
{
	return EPlaybackMode::PlayingForward;
}

bool SSkillSingleActionScrubPanel::IsRecording() const
{
	return false;
}

bool SSkillSingleActionScrubPanel::IsLoopStatusOn() const
{
	return false;
}

#undef LOCTEXT_NAMESPACE