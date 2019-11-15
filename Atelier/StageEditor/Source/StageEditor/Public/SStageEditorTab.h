// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-16

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "DeclarativeSyntaxSupport.h"
#include "StageTable.h"
#include "SBorder.h"
#include "GCObject.h"

class IStageEditorInterface;
class SStageModeBase;

class SStageEditorTab : public SCompoundWidget, public FGCObject
{
public:
    SLATE_BEGIN_ARGS(SStageEditorTab) {}
    SLATE_END_ARGS();

	virtual ~SStageEditorTab();

	void Construct(const FArguments& InArgs, IStageEditorInterface* Proxy);

	void RegisterMode(TSharedPtr<SStageModeBase> InMode, bool bInitSelected = false);

	void SetEditingStageScope(FStageScopePtr Stage);
	FStageScopePtr GetEditingStageScopePtr() { return EditingStageScopePtr; }

private:	
	void RegistModes();

	/** open StageEdLevel */
	FReply OnClickOpenStageEdLevel();
	/** if StageEdLevel is opened */
	bool IsStageEdLevelValid();
	bool IsStageEdContextValid();

	/** Get World */
	UWorld* GetWorld();

	void OnBeginPIE(const bool bIsSimulating);
	void OnEndPIE(const bool bIsSimulating);

	void BuildBaseToolBar();

	void SaveStage();

	/** Update toolbar & content for selection mode */
	void UpdateSelectionMode(TSharedPtr<SStageModeBase> InMode);

	bool IsActionChecked(TSharedPtr<SStageModeBase> InlineContent);

	/** Creates and sets the mode toolbar */
	void UpdateModeToolBar();

	// ~Begin: FGCObject Interface
	void AddReferencedObjects(FReferenceCollector& Collector) override;
	// ~End: FGCObject Interface

private:
    /** Proxy */
	IStageEditorInterface* Proxy;

	/** The container holding the mode toolbar */
	TSharedPtr<SBorder> BaseToolBarContainer;

	/** Inline content area for editor modes */
	TSharedPtr<SBorder> InlineContentHolder;

	/** The container holding the mode toolbar */
	TSharedPtr<SBorder> ModeToolBarContainer;

	TArray<TSharedPtr<SStageModeBase>> StageModeList;

	TSharedPtr<SStageModeBase> CurrentSelectedStageMode;

	FStageScopePtr EditingStageScopePtr;
};