// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-20

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "SGraphPalette.h"

/*********************************************************************/
// SActionSkillPaletteItem
/*********************************************************************/
class SActionSkillPaletteItem : public SGraphPaletteItem
{
public:
	SLATE_BEGIN_ARGS( SActionSkillPaletteItem ) {};
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData);

private:
	virtual FText GetItemTooltip() const override;
};

DECLARE_DELEGATE_OneParam(FOnSkillSelected, TSharedPtr<struct FActionSkillScope>/*ActionSkill*/);

/*********************************************************************/
// SActionSkillPalette
/*********************************************************************/
class ACTIONSKILLEDITOR_API SActionSkillPalette : public SGraphPalette
{
public:
	SLATE_BEGIN_ARGS( SActionSkillPalette ) {}
	SLATE_EVENT( FOnSkillSelected, OnSkillSelected)
	SLATE_END_ARGS()

	virtual ~SActionSkillPalette();

	void Construct(const FArguments& InArgs, TWeakPtr<class FActionSkillEditor> InEditorPtr);

	virtual TSharedRef<SWidget> OnCreateWidgetForAction(struct FCreateWidgetForActionData* const InCreateData) override;
	virtual void CollectAllActions(FGraphActionListBuilderBase& OutAllActions) override;

private:
	void OnNewSkill(TWeakPtr<struct FActionSkillScope> SkillObj);
	void OnDelSkill(TWeakPtr<struct FActionSkillScope> SkillObj);
	void OnEditingSkillChanged(TWeakPtr<struct FActionSkillScope> SkillObj);
	TSharedPtr<SWidget> OnActionContextMenuOpening();
	void OnActionSelectionChanged(const TArray< TSharedPtr<FEdGraphSchemaAction> >& Actions, ESelectInfo::Type SelectType);

	void NewAndEditSkill();
	void DeleteSelectionSkill();
	void CopySelectionSkill();
	void OnPasterSkill();

	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	bool SupportsKeyboardFocus() const override { return true; }
	TSharedPtr<FUICommandList> UICommandList;
	void BindCommands();
	void BindCommands(const class FActionSkillPaletteCommands& ActionCommands);
private:
	TWeakPtr<class FActionSkillEditor> ActionSkillEditor;

	/** Delegate to call when skill is selected */
	FOnSkillSelected OnSkillSelected;

	/** Skill Category Names */
	TArray< TSharedPtr<FString> > CategoryNames;

	/** Combo box used to select category */
	TSharedPtr<class STextComboBox> CategoryComboBox;
};