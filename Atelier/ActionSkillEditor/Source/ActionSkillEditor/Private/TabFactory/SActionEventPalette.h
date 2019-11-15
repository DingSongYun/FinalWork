#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "SGraphPalette.h"

class SActionEventPaletteItem : public SGraphPaletteItem
{
public:
	SLATE_BEGIN_ARGS(SActionEventPaletteItem) {};
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData);

private:
	virtual FText GetItemTooltip() const override;
};

DECLARE_DELEGATE_OneParam(FOnEventSelected, TSharedPtr<struct FActionEvent>/*ActionEvent*/);

class ACTIONSKILLEDITOR_API SActionEventPalette : public SGraphPalette
{
public:
	SLATE_BEGIN_ARGS(SActionEventPalette) {}
	SLATE_EVENT(FOnEventSelected, OnEventSelected)
	SLATE_END_ARGS()

	virtual ~SActionEventPalette();

	void Construct(const FArguments& InArgs, TWeakPtr<class FActionSkillEditor> InEditorPtr);

	virtual TSharedRef<SWidget> OnCreateWidgetForAction(struct FCreateWidgetForActionData* const InCreateData) override;
	virtual void CollectAllActions(FGraphActionListBuilderBase& OutAllActions) override;

private:
	void OnNewEvent(TWeakPtr<struct FActionEvent> EventObj);
	void OnDelEvent(TWeakPtr<struct FActionEvent> EventObj);
	bool FilterActionEventStruct(const FAssetData& AssetData);
	template<typename FunctorType> void CreateEventStructPicker(FMenuBuilder& MenuBuilder, FunctorType&& OnPickEventStructFunc);
	void OnActionEventAssetSelected(const FAssetData& AssetData);
	void AddNewEvent(FString NewNotifyName, UScriptStruct* NotifyStruct);
	
	void CopySelectionEvent();
	void PasteSelectionEvent();
	void DeleteSelectionEvent();

	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	bool SupportsKeyboardFocus() const override { return true; }
	TSharedPtr<FUICommandList> UICommandList;
	void BindCommands();
	void BindCommands(const class FActionEventPaletteCommands& ActionCommands);
	TSharedPtr<SWidget> OnActionContextMenuOpening();
	void OnActionSelectionChanged(const TArray< TSharedPtr<FEdGraphSchemaAction> >& Actions, ESelectInfo::Type SelectType);

private:
	TWeakPtr<class FActionSkillEditor> ActionSkillEditor;

	/** Delegate to call when skill is selected */
	FOnEventSelected OnEventSelected;

	/** Event Category Names */
	TArray< TSharedPtr<FString> > CategoryNames;

	/** Combo box used to select category */
	TSharedPtr<class STextComboBox> CategoryComboBox;
};
