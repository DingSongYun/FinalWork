// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2018-06-29

#pragma once

#include "EditorTool.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SCompoundWidget.h"
#include "AssetData.h"
#include "Widgets/Views/STableRow.h"
#include "System/Project.h"
#include "WorkshopEditorTool.h"

/*
class SPanel;
class SButton;
class SWrapBox;
class SSplitter;
class STextBlock;
struct FAssetData;
class UActorFactory;
*/

DECLARE_LOG_CATEGORY_EXTERN(LogAtelierWorkshopEditor, Log, All);


class SWorkshopEditorToolbar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWorkshopEditorToolbar) {}
		SLATE_ARGUMENT(TArray<FEditorTool*>, ToolsToRegister)
	SLATE_END_ARGS()

	SWorkshopEditorToolbar();
	~SWorkshopEditorToolbar();

	void Construct(const FArguments& InArgs);

	void CleanAll();
	void InitEditorTool();

	class UWorld* GetWorld();
private:

	// Begin SWidget
	virtual void Tick(  const FGeometry& AllottedGeometry,const double InCurrentTime, const float InDeltaTime ) override;
	// End SWidget

	UPROPERTY()
	class AWorkshopEditorTool* EditorTool;
};