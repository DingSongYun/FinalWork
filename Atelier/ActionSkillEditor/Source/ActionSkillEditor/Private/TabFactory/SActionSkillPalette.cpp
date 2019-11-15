// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-20
#include "SActionSkillPalette.h"
#include "Widgets/Input/STextComboBox.h"
#include "EditorStyleSet.h"
#include "ActionSkillGraphSchema.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Serialization/SerializeUtils.h"
#include "ActionSkillEditor.h"
#include "ActionSkill.h"
#include "Framework/Commands/Commands.h"

#define LOCTEXT_NAMESPACE "ActionSkillPalette"

class FActionSkillPaletteCommands : public TCommands<FActionSkillPaletteCommands>
{
public:
	FActionSkillPaletteCommands() : TCommands<FActionSkillPaletteCommands>("ActionSkillPalette", FText::FromString("Action Skill Palette"), NAME_None, FEditorStyle::GetStyleSetName())
	{

	}

	TSharedPtr<FUICommandInfo> DeleteSkill;

	virtual void RegisterCommands() override
	{
		UI_COMMAND(DeleteSkill, "Delete", "Deletes the selected skill.", EUserInterfaceActionType::Button, FInputChord(EKeys::Platform_Delete));
	}
};

/*********************************************************************/
// SActionSkillPaletteItem
/*********************************************************************/
void SActionSkillPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData)
{
	FSlateFontInfo NameFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);

	check(InCreateData->Action.IsValid());

	TSharedPtr<FEdGraphSchemaAction> GraphAction = InCreateData->Action;
	ActionPtr = InCreateData->Action;

	const FSlateBrush* IconBrush = FEditorStyle::GetBrush(TEXT("NoBrush"));
	FSlateColor IconColor = FSlateColor::UseForeground();
	FText IconToolTip = GraphAction->GetTooltipDescription();
	bool bIsReadOnly = false;

	TSharedRef<SWidget> IconWidget = CreateIconWidget( IconToolTip, IconBrush, IconColor );
	TSharedRef<SWidget> NameSlotWidget = CreateTextSlotWidget( NameFont, InCreateData, bIsReadOnly );
	//TSharedRef<SWidget> HotkeyDisplayWidget = CreateHotkeyDisplayWidget( NameFont, HotkeyChord );

	// Create the actual widget
	this->ChildSlot
	[
		SNew(SHorizontalBox)
		// Icon slot
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			IconWidget
		]
		// Name slot
		+SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(3,0)
		[
			NameSlotWidget
		]
	];
}

FText SActionSkillPaletteItem::GetItemTooltip() const
{
	return ActionPtr.Pin()->GetTooltipDescription();
}

/*********************************************************************/
// SActionSkillPalette
/*********************************************************************/
SActionSkillPalette::~SActionSkillPalette()
{
	//ActionSkillEditor.Pin()->OnNewSkill().Remove(this, SActionSkillPalette::OnNewOrDelSkill);
	//ActionSkillEditor.Pin()->OnDeleteSkill().Rem(this, SActionSkillPalette::OnNewOrDelSkill);
}

void SActionSkillPalette::Construct(const FArguments& InArgs, TWeakPtr<class FActionSkillEditor> InEditorPtr)
{
	ActionSkillEditor = InEditorPtr;
	OnSkillSelected = InArgs._OnSkillSelected;

	CategoryNames.Add(MakeShareable(new FString("All")));

	check(ActionSkillEditor.IsValid());

	ActionSkillEditor.Pin()->OnNewSkill().AddRaw(this, &SActionSkillPalette::OnNewSkill);
	ActionSkillEditor.Pin()->OnDeleteSkill().AddRaw(this, &SActionSkillPalette::OnDelSkill);

	this->ChildSlot
	[
		SNew(SBorder)
		.Padding(2.0f)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Category", "Category: "))
				]
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SAssignNew(CategoryComboBox, STextComboBox)
					.OptionsSource(&CategoryNames)
					.InitiallySelectedItem(CategoryNames[0])
				]
			]
			+SVerticalBox::Slot()
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					// Old Expression and Function lists were auto expanded so do the same here for now
					SAssignNew(GraphActionMenu, SGraphActionMenu)
					.OnActionSelected(this, &SActionSkillPalette::OnActionSelectionChanged)
					.OnCreateWidgetForAction(this, &SActionSkillPalette::OnCreateWidgetForAction)
					.OnContextMenuOpening(this, &SActionSkillPalette::OnActionContextMenuOpening)
					.OnCollectAllActions(this, &SActionSkillPalette::CollectAllActions)
					.AutoExpandActionMenu(true)
				]
			]
		]
	];

	BindCommands();
}


FReply SActionSkillPalette::OnKeyDown(const FGeometry & MyGeometry, const FKeyEvent & InKeyEvent)
{
	if (UICommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SActionSkillPalette::BindCommands()
{
	if (UICommandList.IsValid()) return;
	UICommandList = MakeShareable(new FUICommandList);
	FActionSkillPaletteCommands::Register();
	BindCommands(FActionSkillPaletteCommands::Get());
}

void SActionSkillPalette::BindCommands(const FActionSkillPaletteCommands & ActionCommands)
{
	UICommandList->MapAction(ActionCommands.DeleteSkill, FExecuteAction::CreateSP(this, &SActionSkillPalette::DeleteSelectionSkill));
}

TSharedRef<SWidget> SActionSkillPalette::OnCreateWidgetForAction(struct FCreateWidgetForActionData* const InCreateData)
{
	return	SNew(SActionSkillPaletteItem, InCreateData);
}

void SActionSkillPalette::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	if (const UActionSkillTable* SkillTable = ActionSkillEditor.Pin()->GetSkillTable())
	{
		for (auto It = SkillTable->CreateConstIterator(); It; ++It)
		{
			TSharedPtr<FSkillGraphSchemaAction> SkillSchema(new FSkillGraphSchemaAction(It->Value));
			OutAllActions.AddAction(SkillSchema);
		}
	}
}

void SActionSkillPalette::OnNewSkill(TWeakPtr<struct FActionSkillScope> SkillObj)
{
 	RefreshActionsList(true);
	const FName skillname = FName(*(SkillObj.Pin()->GetName() + "(" + FString::FromInt(SkillObj.Pin()->GetId()) + (SkillObj.Pin()->IsDirty() ? ") *" : ")")));
	GraphActionMenu->SelectItemByName(skillname, ESelectInfo::Direct);
}

void SActionSkillPalette::OnDelSkill(TWeakPtr<struct FActionSkillScope> SkillObj)
{
	RefreshActionsList(true);
}

void SActionSkillPalette::OnActionSelectionChanged(const TArray< TSharedPtr<FEdGraphSchemaAction> >& Actions, ESelectInfo::Type SelectType)
{
	if (Actions.Num())
	{
		if (FSkillGraphSchemaAction* SelectedAction = static_cast<FSkillGraphSchemaAction*>(Actions[0].Get()))
		{
			OnSkillSelected.Execute(SelectedAction->ActionSkillPtr);
		}
	}
	else
	{
		OnSkillSelected.Execute(nullptr);
	}
}

TSharedPtr<SWidget> SActionSkillPalette::OnActionContextMenuOpening()
{
	const bool bHasEditingSkill = ActionSkillEditor.Pin()->GetEditingSkill().IsValid();

	if (bHasEditingSkill)
	{
		const bool bInShouldCloseWindowAfterMenuSelection = true;
		FMenuBuilder MenuBuilder(bInShouldCloseWindowAfterMenuSelection, nullptr);
		MenuBuilder.BeginSection("ActionSkill", LOCTEXT("ActionSkill", "ActionSkill"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ActionSkill_CopySkill_Lable", "Copy Skill..."),
				LOCTEXT("ActionSkill_CopySkill_ToolTip", "Copy Skill"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &SActionSkillPalette::CopySelectionSkill))
			);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ActionSkill_DeleteSkill_Lable", "Delete Skill..."),
				LOCTEXT("ActionSkill_DeleteSkill_ToolTip", "Delete Skill"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &SActionSkillPalette::DeleteSelectionSkill))
			);
		}
		MenuBuilder.EndSection();
		return MenuBuilder.MakeWidget();
	}
	else
	{
		const bool bInShouldCloseWindowAfterMenuSelection = true;
		FMenuBuilder MenuBuilder(bInShouldCloseWindowAfterMenuSelection, nullptr);
		MenuBuilder.BeginSection("ActionSkill", LOCTEXT("ActionSkill", "ActionSkill"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ActionSkill_NewSkill_Lable", "New Skill..."),
				LOCTEXT("ActionSkill_NewSkill_ToolTip", "New Skill"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &SActionSkillPalette::NewAndEditSkill))
			);
			FString PasterString;
			FPlatformApplicationMisc::ClipboardPaste(PasterString);
			if (PasterString.Contains("CopiedSkill_"))
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("ActionSkill_PasterSkill_Lable", "Paster Skill..."),
					LOCTEXT("ActionSkill_PasterSkill_ToolTip", "Paster Skill"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateRaw(this, &SActionSkillPalette::OnPasterSkill))
				);
			}
		}
		MenuBuilder.EndSection();
		return MenuBuilder.MakeWidget();
	}
}

void SActionSkillPalette::NewAndEditSkill()
{
	ActionSkillEditor.Pin()->NewAndEditSkill(); 
}

void SActionSkillPalette::DeleteSelectionSkill()
{
	TSharedPtr<FActionSkillScope>& CurrSkill = ActionSkillEditor.Pin()->GetEditingSkill();
	if (CurrSkill.IsValid())
	{
		ActionSkillEditor.Pin()->DeleteAndEditSkill(CurrSkill);
	}
}

void SActionSkillPalette::CopySelectionSkill()
{
	TSharedPtr<FActionSkillScope>& CurrSkill = ActionSkillEditor.Pin()->GetEditingSkill();
	if (CurrSkill.IsValid())
	{
		FString CopyString;
		if (StructSerializer::ExportString(CopyString, Cast<UScriptStruct>(CurrSkill->GetData()->GetStruct()), CurrSkill->GetData()->GetStructMemory(), PPF_None))
		{
			FPlatformApplicationMisc::ClipboardCopy(*("CopiedSkill_" + CopyString));
		}
	}
}

void SActionSkillPalette::OnPasterSkill()
{
	FString PasterString;
	FPlatformApplicationMisc::ClipboardPaste(PasterString);
	TSharedPtr<FActionSkillScope> NewSkill = ActionSkillEditor.Pin()->NewAndEditSkill();
	int32 NewId = NewSkill->GetId();
	FSerializeResult ImportResult;
	UScriptStruct* Struct = const_cast<UScriptStruct*>(Cast<UScriptStruct>(NewSkill->GetData()->GetStruct()));
	PasterString = PasterString.Replace(TEXT("CopiedSkill_"), TEXT(""));
	if (StructSerializer::ImportString(PasterString, Struct, NewSkill->GetData()->GetStructMemory(), PPF_None, Struct->GetName(), ImportResult))
	{
		NewSkill->SetId(NewId);
		NewSkill->SetName(NewSkill->GetName() + " Copy");
		NewSkill->MarkDirty(true);
	}
}
#undef LOCTEXT_NAMESPACE
