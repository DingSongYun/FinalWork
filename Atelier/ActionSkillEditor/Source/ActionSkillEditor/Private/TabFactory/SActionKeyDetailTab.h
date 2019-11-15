// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-08-12

#pragma once
#include "CoreMinimal.h"
#include "Misc/NotifyHook.h"
#include "SSingleObjectDetailsPanel.h"
#include "ActionEvent.h"

struct FActionKeyFrame;

class SActionKeyDetailTab : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SActionKeyDetailTab) {}

	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs, TWeakPtr<class IActionSkillEditor> InEditorPtr);

	//~ Begin: FNotifyHook
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged) override;
	//~ End: FNotifyHook
private:
	void OnEditingKeyFrameChanged(FActionKeyFrame* NewKeyFrame);
	void OnEditingEventChanged(TWeakPtr<FActionEvent> NewEventPtr);
	void UpdateActionKeyDetail();
	void UpdateActionEventDetail();
	void UpdateActionEventKeyFrameDetail();
	TSharedRef<class IPropertyTypeCustomization> CreateKeyNotifyCustomizationLayout();
private:
	/** Pointer to ActionSkillEditor. */
	TWeakPtr<class IActionSkillEditor> ActionSkillEditor;

	/** Widget to show ActionSkill data. */
	TSharedPtr<class IStructureDetailsView> KeyPropertyView;
	TWeakPtr<struct FActionSkillScope> EditingSkill;
	TWeakPtr<struct FActionEvent> EditingEvent;
};