// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-27

#pragma once
#include "CoreMinimal.h"
#include "Misc/NotifyHook.h"
#include "SSingleObjectDetailsPanel.h"

class SActionSkillDetailTab : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SActionSkillDetailTab) {}

	SLATE_END_ARGS()

public:
	/** Destructor */
	~SActionSkillDetailTab();

	void Construct(const FArguments& InArgs, TWeakPtr<class IActionSkillEditor> InEditorPtr);

	//~ Begin: FNotifyHook
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged) override;
	//~ End: FNotifyHook
private:
	/** When change skill. */
	void OnEditingSkillChanged(TWeakPtr<struct FActionSkillScope> NewSkill);
	void OnEditingEventChanged(TWeakPtr<struct FActionEvent> NewEvent);
	/** Handles checking whether the details view is editable. */
	bool HandleDetailsViewIsPropertyEditable() const;

	/** Handles determining the visibility of the details view. */
	EVisibility HandleDetailsViewVisibility() const;

	/** Set ActionSkill data to IStructureDetailsView. */
	void UpdateSkillDetail();
private:
	/** Pointer to ActionSkillEditor. */
	TWeakPtr<class IActionSkillEditor> ActionSkillEditor;

	/** Widget to show ActionSkill data. */
	TSharedPtr<class IStructureDetailsView> PropertyView;
};
