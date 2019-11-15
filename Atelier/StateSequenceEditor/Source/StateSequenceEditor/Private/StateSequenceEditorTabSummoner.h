// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-31

#pragma once

#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"

class FBlueprintEditor;
class SStateSequenceEditorWidgetImpl;
class UStateSequence;
class UStateSequenceComponent;


class STATESEQUENCEEDITOR_API SStateSequenceEditorWidget
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SStateSequenceEditorWidget){}
	SLATE_END_ARGS();

	void Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor);
	void Initialize(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UStateSequence* NewStateSequence);
	UStateSequence* GetSequence() const;
	FText GetDisplayLabel() const;

private:

	TWeakPtr<SStateSequenceEditorWidgetImpl> Impl;
};


struct STATESEQUENCEEDITOR_API FStateSequenceEditorSummoner
	: public FWorkflowTabFactory
{
	FStateSequenceEditorSummoner(TSharedPtr<FBlueprintEditor> BlueprintEditor);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;

protected:
	TWeakPtr<FBlueprintEditor> WeakBlueprintEditor;
};
