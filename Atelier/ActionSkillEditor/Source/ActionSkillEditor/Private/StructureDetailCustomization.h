#pragma once

#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "PropertyCustomizationHelpers.h"
#include "Kismet2/StructureEditorUtils.h"
#include "IDetailCustomNodeBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "SPinTypeSelector.h"

#define LOCTEXT_NAMESPACE "ActionSkillEditor"

enum EMemberFieldPosition
{
	MFP_First	=	0x1,
	MFP_Last	=	0x2,
};

class FUserDefinedStructureDetails : public IDetailCustomization, FStructureEditorUtils::INotifyOnStructChanged
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FUserDefinedStructureDetails);
	}

	~FUserDefinedStructureDetails()
	{
	}

	UUserDefinedStruct* GetUserDefinedStruct()
	{
		return UserDefinedStruct.Get();
	}

	struct FStructVariableDescription* FindStructureFieldByGuid(FGuid Guid)
	{
		if (auto Struct = GetUserDefinedStruct())
		{
			return FStructureEditorUtils::GetVarDesc(Struct).FindByPredicate(FStructureEditorUtils::FFindByGuidHelper<FStructVariableDescription>(Guid));
		}
		return NULL;
	}

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailLayout) override;

	/** FStructureEditorUtils::INotifyOnStructChanged */
	virtual void PreChange(const class UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info) override {}
	virtual void PostChange(const class UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info) override;

private:
	FReply OnClickRenameStructure(TWeakObjectPtr<UObject> InObject);
	FReply OnOpenStructEditor();

private:
	TWeakObjectPtr<UUserDefinedStruct> UserDefinedStruct;
	TSharedPtr<class FUserDefinedStructureLayout> Layout;
};

class FUserDefinedStructureLayout : public IDetailCustomNodeBuilder, public TSharedFromThis<FUserDefinedStructureLayout>
{
public:
	FUserDefinedStructureLayout(TWeakPtr<class FUserDefinedStructureDetails> InStructureDetails)
		: StructureDetails(InStructureDetails)
		, InitialPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType())
	{}

	void OnChanged()
	{
		OnRegenerateChildren.ExecuteIfBound();
	}

	FReply OnAddNewField()
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if(StructureDetailsSP.IsValid())
		{
			FStructureEditorUtils::AddVariable(StructureDetailsSP->GetUserDefinedStruct(), InitialPinType);
		}

		return FReply::Handled();
	}

	const FSlateBrush* OnGetStructureStatus() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if(StructureDetailsSP.IsValid())
		{
			if(auto Struct = StructureDetailsSP->GetUserDefinedStruct())
			{
				switch(Struct->Status.GetValue())
				{
				case EUserDefinedStructureStatus::UDSS_Error:
					return FEditorStyle::GetBrush("Kismet.Status.Error.Small");
				case EUserDefinedStructureStatus::UDSS_UpToDate:
					return FEditorStyle::GetBrush("Kismet.Status.Good.Small");
				default:
					return FEditorStyle::GetBrush("Kismet.Status.Unknown.Small");
				}
				
			}
		}
		return NULL;
	}

	FText GetStatusTooltip() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			if (auto Struct = StructureDetailsSP->GetUserDefinedStruct())
			{
				switch (Struct->Status.GetValue())
				{
				case EUserDefinedStructureStatus::UDSS_Error:
					return FText::FromString(Struct->ErrorMessage);
				}
			}
		}
		return FText::GetEmpty();
	}

	FText OnGetTooltipText() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			if (auto Struct = StructureDetailsSP->GetUserDefinedStruct())
			{
				return FText::FromString(FStructureEditorUtils::GetTooltip(Struct));
			}
		}
		return FText();
	}

	void OnTooltipCommitted(const FText& NewText, ETextCommit::Type InTextCommit)
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			if (auto Struct = StructureDetailsSP->GetUserDefinedStruct())
			{
				FStructureEditorUtils::ChangeTooltip(Struct, NewText.ToString());
			}
		}
	}

	/** Callback when a pin type is selected to cache the value so new variables in the struct will be set to the cached type */
	void OnPinTypeSelected(const FEdGraphPinType& InPinType)
	{
		InitialPinType = InPinType;
	}

	/** IDetailCustomNodeBuilder Interface*/
	virtual void SetOnRebuildChildren( FSimpleDelegate InOnRegenerateChildren ) override 
	{
		OnRegenerateChildren = InOnRegenerateChildren;
	}

	virtual void GenerateChildContent( IDetailChildrenBuilder& ChildrenBuilder ) override;

	virtual void GenerateHeaderRowContent( FDetailWidgetRow& NodeRow ) override {}

	virtual void Tick( float DeltaTime ) override {}
	virtual bool RequiresTick() const override { return false; }
	virtual FName GetName() const override 
	{ 
		auto StructureDetailsSP = StructureDetails.Pin();
		if(StructureDetailsSP.IsValid())
		{
			if(auto Struct = StructureDetailsSP->GetUserDefinedStruct())
			{
				return Struct->GetFName();
			}
		}
		return NAME_None; 
	}
	virtual bool InitiallyCollapsed() const override { return false; }

private:
	TWeakPtr<class FUserDefinedStructureDetails> StructureDetails;
	FSimpleDelegate OnRegenerateChildren;

	/** Cached value of the last pin type the user selected, used as the initial value for new struct members */
	FEdGraphPinType InitialPinType;
};

class FUserDefinedStructureFieldLayout : public IDetailCustomNodeBuilder, public TSharedFromThis<FUserDefinedStructureFieldLayout>
{
public:
	FUserDefinedStructureFieldLayout(TWeakPtr<class FUserDefinedStructureDetails> InStructureDetails, TWeakPtr<class FUserDefinedStructureLayout> InStructureLayout, FGuid InFieldGuid, uint32 InPositionFlags)
		: StructureDetails(InStructureDetails)
		, StructureLayout(InStructureLayout)
		, FieldGuid(InFieldGuid)
		, PositionFlags(InPositionFlags) {}

	void OnChanged()
	{
		OnRegenerateChildren.ExecuteIfBound();
	}

	FText OnGetNameText() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if(StructureDetailsSP.IsValid())
		{
			return FText::FromString(FStructureEditorUtils::GetVariableDisplayName(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid));
		}
		return FText::GetEmpty();
	}

	void OnNameTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit)
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if(StructureDetailsSP.IsValid())
		{
			const FString NewNameStr = NewText.ToString();
			FStructureEditorUtils::RenameVariable(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid, NewNameStr);
		}
	}

	FEdGraphPinType OnGetPinInfo() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if(StructureDetailsSP.IsValid())
		{
			if(const FStructVariableDescription* FieldDesc = StructureDetailsSP->FindStructureFieldByGuid(FieldGuid))
			{
				return FieldDesc->ToPinType();
			}
		}
		return FEdGraphPinType();
	}

	void PinInfoChanged(const FEdGraphPinType& PinType)
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if(StructureDetailsSP.IsValid())
		{
			FStructureEditorUtils::ChangeVariableType(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid, PinType);
			auto StructureLayoutPin = StructureLayout.Pin();
			if (StructureLayoutPin.IsValid())
			{
				StructureLayoutPin->OnPinTypeSelected(PinType);
			}
		}
	}

	void OnPrePinInfoChange(const FEdGraphPinType& PinType)
	{

	}

	void OnRemovField()
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if(StructureDetailsSP.IsValid())
		{
			FStructureEditorUtils::RemoveVariable(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid);
		}
	}

	bool IsRemoveButtonEnabled()
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			if (auto UDStruct = StructureDetailsSP->GetUserDefinedStruct())
			{
				return (FStructureEditorUtils::GetVarDesc(UDStruct).Num() > 1);
			}
		}
		return false;
	}

	FText OnGetTooltipText() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			if (const FStructVariableDescription* FieldDesc = StructureDetailsSP->FindStructureFieldByGuid(FieldGuid))
			{
				return FText::FromString(FieldDesc->ToolTip);
			}
		}
		return FText();
	}

	void OnTooltipCommitted(const FText& NewText, ETextCommit::Type InTextCommit)
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			FStructureEditorUtils::ChangeVariableTooltip(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid, NewText.ToString());
		}
	}

	ECheckBoxState OnGetEditableOnBPInstanceState() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			if (const FStructVariableDescription* FieldDesc = StructureDetailsSP->FindStructureFieldByGuid(FieldGuid))
			{
				return !FieldDesc->bDontEditoOnInstance ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			}
		}
		return ECheckBoxState::Undetermined;
	}

	void OnEditableOnBPInstanceCommitted(ECheckBoxState InNewState)
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			FStructureEditorUtils::ChangeEditableOnBPInstance(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid, ECheckBoxState::Unchecked != InNewState);
		}
	}

	// Multi-line text
	EVisibility IsMultiLineTextOptionVisible() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			return FStructureEditorUtils::CanEnableMultiLineText(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid) ? EVisibility::Visible : EVisibility::Collapsed;
		}
		return EVisibility::Collapsed;
	}

	ECheckBoxState OnGetMultiLineTextEnabled() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			return FStructureEditorUtils::IsMultiLineTextEnabled(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		}
		return ECheckBoxState::Undetermined;
	}

	void OnMultiLineTextEnabledCommitted(ECheckBoxState InNewState)
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid() && (ECheckBoxState::Undetermined != InNewState))
		{
			FStructureEditorUtils::ChangeMultiLineTextEnabled(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid, ECheckBoxState::Checked == InNewState);
		}
	}

	// 3D widget
	EVisibility Is3dWidgetOptionVisible() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			return FStructureEditorUtils::CanEnable3dWidget(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid) ? EVisibility::Visible : EVisibility::Collapsed;
		}
		return EVisibility::Collapsed;
	}

	ECheckBoxState OnGet3dWidgetEnabled() const
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid())
		{
			return FStructureEditorUtils::Is3dWidgetEnabled(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		}
		return ECheckBoxState::Undetermined;
	}

	void On3dWidgetEnabledCommitted(ECheckBoxState InNewState)
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid() && (ECheckBoxState::Undetermined != InNewState))
		{
			FStructureEditorUtils::Change3dWidgetEnabled(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid, ECheckBoxState::Checked == InNewState);
		}
	}

	/** IDetailCustomNodeBuilder Interface*/
	virtual void SetOnRebuildChildren( FSimpleDelegate InOnRegenerateChildren ) override 
	{
		OnRegenerateChildren = InOnRegenerateChildren;
	}

	EVisibility GetErrorIconVisibility()
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if(StructureDetailsSP.IsValid())
		{
			auto FieldDesc = StructureDetailsSP->FindStructureFieldByGuid(FieldGuid);
			if (FieldDesc && FieldDesc->bInvalidMember)
			{
				return EVisibility::Visible;
			}
		}

		return EVisibility::Collapsed;
	}

	void RemoveInvalidSubTypes(TSharedPtr<UEdGraphSchema_K2::FPinTypeTreeInfo> PinTypeNode, const UUserDefinedStruct* Parent) const
	{
		if (!PinTypeNode.IsValid() || !Parent)
		{
			return;
		}

		for (int32 ChildIndex = 0; ChildIndex < PinTypeNode->Children.Num();)
		{
			const auto Child = PinTypeNode->Children[ChildIndex];
			if(Child.IsValid())
			{
				const bool bCanCheckSubObjectWithoutLoading = Child->GetPinType(false).PinSubCategoryObject.IsValid();
				if (bCanCheckSubObjectWithoutLoading && !FStructureEditorUtils::CanHaveAMemberVariableOfType(Parent, Child->GetPinType(false)))
				{
					PinTypeNode->Children.RemoveAt(ChildIndex);
					continue;
				}
			}
			++ChildIndex;
		}
	}

	void GetFilteredVariableTypeTree( TArray< TSharedPtr<UEdGraphSchema_K2::FPinTypeTreeInfo> >& TypeTree, ETypeTreeFilter TypeTreeFilter) const
	{
		auto K2Schema = GetDefault<UEdGraphSchema_K2>();
		auto StructureDetailsSP = StructureDetails.Pin();
		if(StructureDetailsSP.IsValid() && K2Schema)
		{
			K2Schema->GetVariableTypeTree(TypeTree, TypeTreeFilter);
			const auto Parent = StructureDetailsSP->GetUserDefinedStruct();
			// THE TREE HAS ONLY 2 LEVELS
			for (auto PinTypePtr : TypeTree)
			{
				RemoveInvalidSubTypes(PinTypePtr, Parent);
			}
		}
	}

	FReply OnMoveUp()
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid() && !(PositionFlags & EMemberFieldPosition::MFP_First))
		{
			FStructureEditorUtils::MoveVariable(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid, FStructureEditorUtils::MD_Up);
		}
		return FReply::Handled();
	}

	FReply OnMoveDown()
	{
		auto StructureDetailsSP = StructureDetails.Pin();
		if (StructureDetailsSP.IsValid() && !(PositionFlags & EMemberFieldPosition::MFP_Last))
		{
			FStructureEditorUtils::MoveVariable(StructureDetailsSP->GetUserDefinedStruct(), FieldGuid, FStructureEditorUtils::MD_Down);
		}
		return FReply::Handled();
	}

	virtual void GenerateHeaderRowContent( FDetailWidgetRow& NodeRow ) override 
	{
		auto K2Schema = GetDefault<UEdGraphSchema_K2>();

		TSharedPtr<SImage> ErrorIcon;

		const float ValueContentWidth = 250.0f;

		NodeRow
		.NameContent()
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SAssignNew(ErrorIcon, SImage)
				.Image( FEditorStyle::GetBrush("Icons.Error") )
			]

			+SHorizontalBox::Slot()
			.FillWidth(1)
			.VAlign(VAlign_Center)
			[
				SNew(SEditableTextBox)
				.Text( this, &FUserDefinedStructureFieldLayout::OnGetNameText )
				.OnTextCommitted( this, &FUserDefinedStructureFieldLayout::OnNameTextCommitted )
				.Font( IDetailLayoutBuilder::GetDetailFont() )
			]
		]
		.ValueContent()
		.MaxDesiredWidth(ValueContentWidth)
		.MinDesiredWidth(ValueContentWidth)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SPinTypeSelector, FGetPinTypeTree::CreateSP(this, &FUserDefinedStructureFieldLayout::GetFilteredVariableTypeTree))
				.TargetPinType(this, &FUserDefinedStructureFieldLayout::OnGetPinInfo)
				.OnPinTypePreChanged(this, &FUserDefinedStructureFieldLayout::OnPrePinInfoChange)
				.OnPinTypeChanged(this, &FUserDefinedStructureFieldLayout::PinInfoChanged)
				.Schema(K2Schema)
				.TypeTreeFilter(ETypeTreeFilter::None)
				.Font( IDetailLayoutBuilder::GetDetailFont() )
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ContentPadding(0)
				.OnClicked(this, &FUserDefinedStructureFieldLayout::OnMoveUp)
				.IsEnabled(!(EMemberFieldPosition::MFP_First & PositionFlags))
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("BlueprintEditor.Details.ArgUpButton"))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ContentPadding(0)
				.OnClicked(this, &FUserDefinedStructureFieldLayout::OnMoveDown)
				.IsEnabled(!(EMemberFieldPosition::MFP_Last & PositionFlags))
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("BlueprintEditor.Details.ArgDownButton"))
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				PropertyCustomizationHelpers::MakeClearButton(
					FSimpleDelegate::CreateSP(this, &FUserDefinedStructureFieldLayout::OnRemovField),
					LOCTEXT("RemoveVariable", "Remove member variable"),
					TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(this, &FUserDefinedStructureFieldLayout::IsRemoveButtonEnabled)))
			]
		];

		if (ErrorIcon.IsValid())
		{
			ErrorIcon->SetVisibility(
				TAttribute<EVisibility>::Create(
					TAttribute<EVisibility>::FGetter::CreateSP(
						this, &FUserDefinedStructureFieldLayout::GetErrorIconVisibility)));
		}
	}

	virtual void GenerateChildContent( IDetailChildrenBuilder& ChildrenBuilder ) override 
	{
		ChildrenBuilder.AddCustomRow(LOCTEXT("Tooltip", "Tooltip"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Tooltip", "Tooltip"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SEditableTextBox)
			.Text(this, &FUserDefinedStructureFieldLayout::OnGetTooltipText)
			.OnTextCommitted(this, &FUserDefinedStructureFieldLayout::OnTooltipCommitted)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		];

		ChildrenBuilder.AddCustomRow(LOCTEXT("EditableOnInstance", "EditableOnInstance"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Editable", "Editable"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SCheckBox)
			.ToolTipText(LOCTEXT("EditableOnBPInstance", "Variable can be edited on an instance of a Blueprint."))
			.OnCheckStateChanged(this, &FUserDefinedStructureFieldLayout::OnEditableOnBPInstanceCommitted)
			.IsChecked(this, &FUserDefinedStructureFieldLayout::OnGetEditableOnBPInstanceState)
		];

		ChildrenBuilder.AddCustomRow(LOCTEXT("MultiLineText", "Multi-line Text"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MultiLineText", "Multi-line Text"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SCheckBox)
			.ToolTipText(LOCTEXT("MultiLineTextToolTip", "Should this property allow multiple lines of text to be entered?"))
			.OnCheckStateChanged(this, &FUserDefinedStructureFieldLayout::OnMultiLineTextEnabledCommitted)
			.IsChecked(this, &FUserDefinedStructureFieldLayout::OnGetMultiLineTextEnabled)
		]
		.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FUserDefinedStructureFieldLayout::IsMultiLineTextOptionVisible)));

		ChildrenBuilder.AddCustomRow(LOCTEXT("3dWidget", "3D Widget"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("3dWidget", "3D Widget"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SCheckBox)
			.OnCheckStateChanged(this, &FUserDefinedStructureFieldLayout::On3dWidgetEnabledCommitted)
			.IsChecked(this, &FUserDefinedStructureFieldLayout::OnGet3dWidgetEnabled)
		]
		.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FUserDefinedStructureFieldLayout::Is3dWidgetOptionVisible)));
	}

	virtual void Tick( float DeltaTime ) override {}
	virtual bool RequiresTick() const override { return false; }
	virtual FName GetName() const override { return FName(*FieldGuid.ToString()); }
	virtual bool InitiallyCollapsed() const override { return true; }

private:
	TWeakPtr<class FUserDefinedStructureDetails> StructureDetails;

	TWeakPtr<class FUserDefinedStructureLayout> StructureLayout;

	FGuid FieldGuid;

	FSimpleDelegate OnRegenerateChildren;

	uint32 PositionFlags;
};

#undef LOCTEXT_NAMESPACE