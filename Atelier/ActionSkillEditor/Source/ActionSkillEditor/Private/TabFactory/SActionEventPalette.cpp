#include "SActionEventPalette.h"
#include "ActionSkillEditor.h"
#include "EditorStyleSet.h"
#include "ActionSkill.h"
#include "ActionEvent.h"
#include "ActionEventGraphSchema.h"
#include "Framework/Commands/Commands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Serialization/SerializeUtils.h"

#define LOCTEXT_NAMESPACE "ActionEventPalette"

class FActionEventPaletteCommands : public TCommands<FActionEventPaletteCommands>
{
public:
	FActionEventPaletteCommands() : TCommands<FActionEventPaletteCommands>("ActionEventPalette", FText::FromString("Action Event Palette"), NAME_None, FEditorStyle::GetStyleSetName())
	{

	}

	TSharedPtr<FUICommandInfo> DeleteSkill;

	virtual void RegisterCommands() override
	{
		UI_COMMAND(DeleteSkill, "Delete", "Deletes the selected event.", EUserInterfaceActionType::Button, FInputChord(EKeys::Platform_Delete));
	}
};

void SActionEventPaletteItem::Construct(const FArguments & InArgs, FCreateWidgetForActionData * const InCreateData)
{
	FSlateFontInfo NameFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);

	check(InCreateData->Action.IsValid());

	TSharedPtr<FEdGraphSchemaAction> GraphAction = InCreateData->Action;
	ActionPtr = InCreateData->Action;

	const FSlateBrush* IconBrush = FEditorStyle::GetBrush(TEXT("NoBrush"));
	FSlateColor IconColor = FSlateColor::UseForeground();
	FText IconToolTip = GraphAction->GetTooltipDescription();
	bool bIsReadOnly = false;

	TSharedRef<SWidget> IconWidget = CreateIconWidget(IconToolTip, IconBrush, IconColor);
	TSharedRef<SWidget> NameSlotWidget = CreateTextSlotWidget(NameFont, InCreateData, bIsReadOnly);
	//TSharedRef<SWidget> HotkeyDisplayWidget = CreateHotkeyDisplayWidget( NameFont, HotkeyChord );

	// Create the actual widget
	this->ChildSlot
	[
		SNew(SHorizontalBox)
		// Icon slot
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			IconWidget
		]
		// Name slot
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(3, 0)
		[
			NameSlotWidget
		]
	];
}

FText SActionEventPaletteItem::GetItemTooltip() const
{
	return ActionPtr.Pin()->GetTooltipDescription();
}

SActionEventPalette::~SActionEventPalette()
{
}

void SActionEventPalette::Construct(const FArguments & InArgs, TWeakPtr<class FActionSkillEditor> InEditorPtr)
{
	ActionSkillEditor = InEditorPtr;
	OnEventSelected = InArgs._OnEventSelected;

	CategoryNames.Add(MakeShareable(new FString("All")));

	check(ActionSkillEditor.IsValid());

	ActionSkillEditor.Pin()->OnNewEvent().AddRaw(this, &SActionEventPalette::OnNewEvent);
	ActionSkillEditor.Pin()->OnDeleteEvent().AddRaw(this, &SActionEventPalette::OnDelEvent);

	this->ChildSlot
	[
		SNew(SBorder)
		.Padding(2.0f)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Category", "Category: "))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SAssignNew(CategoryComboBox, STextComboBox)
					.OptionsSource(&CategoryNames)
					.InitiallySelectedItem(CategoryNames[0])
				]
			]

			// Content list
			+ SVerticalBox::Slot()
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					// Old Expression and Function lists were auto expanded so do the same here for now
					SAssignNew(GraphActionMenu, SGraphActionMenu)
					.OnActionSelected(this, &SActionEventPalette::OnActionSelectionChanged)
					.OnCreateWidgetForAction(this, &SActionEventPalette::OnCreateWidgetForAction)
					.OnContextMenuOpening(this, &SActionEventPalette::OnActionContextMenuOpening)
					.OnCollectAllActions(this, &SActionEventPalette::CollectAllActions)
					.AutoExpandActionMenu(true)
				]
			]
		]
	];

	BindCommands();
}

FReply SActionEventPalette::OnKeyDown(const FGeometry & MyGeometry, const FKeyEvent & InKeyEvent)
{
	if (UICommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SActionEventPalette::BindCommands()
{
	if (UICommandList.IsValid()) return;
	UICommandList = MakeShareable(new FUICommandList);
	FActionEventPaletteCommands::Register();
	BindCommands(FActionEventPaletteCommands::Get());
}

void SActionEventPalette::BindCommands(const FActionEventPaletteCommands & ActionCommands)
{
	UICommandList->MapAction(ActionCommands.DeleteSkill, FExecuteAction::CreateSP(this, &SActionEventPalette::DeleteSelectionEvent));
}

TSharedPtr<SWidget> SActionEventPalette::OnActionContextMenuOpening()
{
	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bInShouldCloseWindowAfterMenuSelection, nullptr);
	MenuBuilder.BeginSection("ActionEvent", LOCTEXT("ActionEvent", "ActionEvent"));
	{
		if (ActionSkillEditor.Pin()->GetEditingEvent().IsValid())
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ActionEvent_CopyEvent_Lable", "Copy Event..."),
				LOCTEXT("ActionEvent_CopyEvent_ToolTip", "Copy Event"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &SActionEventPalette::CopySelectionEvent))
			);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ActionEvent_DeleteEvent_Lable", "Delete Event..."),
				LOCTEXT("ActionEvent_DeleteEvent_ToolTip", "Delete Event"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &SActionEventPalette::DeleteSelectionEvent))
			);
		}
		else 
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("ActionEvent_NewEvent_Lable", "New Event..."),
				LOCTEXT("ActionEvent_NewEvent_ToolTip", "New Event"),
				FNewMenuDelegate::CreateLambda([&](FMenuBuilder& InSubMenuBuilder)
				{
					CreateEventStructPicker(InSubMenuBuilder, [&](const FAssetData& AssetData) {
						OnActionEventAssetSelected(AssetData);
					});
				})
			);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ActionEvent_PasteEvent_Lable", "Paste Event..."),
				LOCTEXT("ActionEvent_PasteEvent_ToolTip", "Paste Event"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &SActionEventPalette::PasteSelectionEvent))
			);
		}
	}
	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void SActionEventPalette::OnActionSelectionChanged(const TArray<TSharedPtr<FEdGraphSchemaAction>>& Actions, ESelectInfo::Type SelectType)
{
	if (Actions.Num())
	{
		if (FEventGraphSchemaAction* SelectedAction = static_cast<FEventGraphSchemaAction*>(Actions[0].Get()))
		{
			OnEventSelected.Execute(SelectedAction->ActionEventPtr);
		}
	}
	else
	{
		OnEventSelected.Execute(nullptr);
	}
}

TSharedRef<SWidget> SActionEventPalette::OnCreateWidgetForAction(FCreateWidgetForActionData * const InCreateData)
{
	return	SNew(SActionEventPaletteItem, InCreateData);
}

void SActionEventPalette::CollectAllActions(FGraphActionListBuilderBase & OutAllActions)
{
	if (const UActionEventTable* EventTable = ActionSkillEditor.Pin()->GetEventTable())
	{
		int count = 0;
		for (auto It = EventTable->CreateConstIterator(); It; ++It)
		{
			count++;
			TSharedPtr<FEventGraphSchemaAction> EventSchema(new FEventGraphSchemaAction(It->Value));
			OutAllActions.AddAction(EventSchema);
		}
	}
}

void SActionEventPalette::OnNewEvent(TWeakPtr<struct FActionEvent> EventObj)
{
	RefreshActionsList(true);
	const FName skillname = FName(*(EventObj.Pin()->Name.ToString() + "(" + FString::FromInt(EventObj.Pin()->Id) + (EventObj.Pin()->IsDirty() ? ") *" : ")")));
	GraphActionMenu->SelectItemByName(skillname, ESelectInfo::Direct);
}

void SActionEventPalette::OnDelEvent(TWeakPtr<struct FActionEvent> EventObj)
{
	RefreshActionsList(true);
}

bool SActionEventPalette::FilterActionEventStruct(const FAssetData & AssetData)
{
	if (UClass* Cls = AssetData.GetClass())
	{
		bool IsActionEventStruct = Cls == UActionEventStructType::StaticClass();
		return !IsActionEventStruct;
	}
	return true;
}

template<typename FunctorType>
inline void SActionEventPalette::CreateEventStructPicker(FMenuBuilder & MenuBuilder, FunctorType && OnPickEventStructFunc)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	FAssetPickerConfig AssetPickerConfig;
	{
		AssetPickerConfig.Filter.ClassNames.Add(UScriptStruct::StaticClass()->GetFName());
		AssetPickerConfig.Filter.bRecursiveClasses = true;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
		AssetPickerConfig.bAllowNullSelection = false;
		AssetPickerConfig.bFocusSearchBoxWhenOpened = true;
		AssetPickerConfig.bAllowDragging = false;
		AssetPickerConfig.SaveSettingsName = TEXT("AssetPropertyPicker");
		AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda(OnPickEventStructFunc);
		AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SActionEventPalette::FilterActionEventStruct);
	}
	MenuBuilder.AddWidget(
		SNew(SBox)
		.MinDesiredWidth(300.0f)
		.MaxDesiredHeight(400.0f)
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		],
		FText(), true, false
	);
}

void SActionEventPalette::OnActionEventAssetSelected(const FAssetData & AssetData)
{
	if (UObject* NotifyStruct = AssetData.GetAsset())
	{
		AddNewEvent(NotifyStruct->GetName(), Cast<UScriptStruct>(NotifyStruct));
	}
	FSlateApplication::Get().DismissAllMenus();
}

void SActionEventPalette::AddNewEvent(FString NewNotifyName, UScriptStruct* NotifyStruct)
{
	if (!NotifyStruct) return;
	FActionEventPtr NewEvent = ActionSkillEditor.Pin()->NewAndEditEvent();
	NewEvent->Name = NotifyStruct->GetFName();
	NewEvent->Params.Initialize(*NotifyStruct);
	NewEvent->OnEventDataDirty.ExecuteIfBound(true);
}

void SActionEventPalette::CopySelectionEvent()
{
	TSharedPtr<FActionEvent>& CurrEventPtr = ActionSkillEditor.Pin()->GetEditingEvent();
	if (CurrEventPtr.IsValid())
	{
		FString CopyString;
		if (StructSerializer::ExportString(CopyString, FActionEvent::StaticStruct(), CurrEventPtr.Get(), PPF_None))
		{
			FPlatformApplicationMisc::ClipboardCopy(*("CopiedEvent_" + CopyString));
		}
	}
}

void SActionEventPalette::PasteSelectionEvent()
{
	FString PasterString;
	FPlatformApplicationMisc::ClipboardPaste(PasterString);
	TSharedPtr<FActionEvent> NewEvent = ActionSkillEditor.Pin()->NewAndEditEvent();
	int32 NewId = NewEvent->Id;
	FSerializeResult ImportResult;
	UScriptStruct* Struct = FActionEvent::StaticStruct();
	PasterString = PasterString.Replace(TEXT("CopiedEvent_"), TEXT(""));
	if (StructSerializer::ImportString(PasterString, Struct, NewEvent.Get(), PPF_None, Struct->GetName(), ImportResult))
	{
		NewEvent->SetId(NewId);
		NewEvent->Name = FName(*(NewEvent->Name.ToString() + " Copy"));
		NewEvent->MarkDirty(true);
		NewEvent->OnEventDataDirty.ExecuteIfBound(true);
	}
}

void SActionEventPalette::DeleteSelectionEvent()
{
	TSharedPtr<FActionEvent>& CurrEvent = ActionSkillEditor.Pin()->GetEditingEvent();
	if (CurrEvent.IsValid())
	{
		ActionSkillEditor.Pin()->DeleteAndEditEvent(CurrEvent);
	}
}

#undef LOCTEXT_NAMESPACE
