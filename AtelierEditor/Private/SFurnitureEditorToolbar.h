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
#include "FurnitureEditorTool.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAtelierFurnitureEditor, Log, All);

struct FAssetData;
struct FGridVolumeObject
{
public:
	FGridVolumeObject(AStaticMeshActor* box)
	{
		this->mBox = TWeakObjectPtr<AStaticMeshActor>(box);
	}
	TWeakObjectPtr<AStaticMeshActor> mBox;
};

class SGridVolumeAssetEntry : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SGridVolumeAssetEntry)
	{}
		SLATE_ARGUMENT(FText, center)
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs, const TSharedPtr<const FGridVolumeObject>& InItem);
	TSharedRef<SVerticalBox> CreateAssetTransformPanel(const TSharedPtr<const FGridVolumeObject>& InItem);
};

class SFurnitureEditorToolbar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFurnitureEditorToolbar) {}
		SLATE_ARGUMENT(TArray<FEditorTool*>, ToolsToRegister)
	SLATE_END_ARGS()

	SFurnitureEditorToolbar();
	~SFurnitureEditorToolbar();

	void Construct(const FArguments& InArgs);

	void CleanAll();
	void InitEditorTool();
	void OnSkeletalMeshSelected(const FAssetData& data);
	void Refresh();
	TSharedRef<ITableRow> OnGenerateWidgetForVolume(TSharedPtr<FGridVolumeObject> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	FString GenGridInfo(TWeakObjectPtr<AStaticMeshActor>);

	class UWorld* GetWorld();
private:

	// Begin SWidget
	virtual void Tick(  const FGeometry& AllottedGeometry,const double InCurrentTime, const float InDeltaTime ) override;
	// End SWidget
	FReply OnClickReimport();
	FReply OnClickSaveVolumn();

	UPROPERTY()
	class AFurnitureEditorTool* EditorTool;
	TSharedPtr<class IDetailsView> DetailedView;
	AActor* Furniture;
	TSharedPtr<SListView<TSharedPtr<FGridVolumeObject>>> GridVolumeListView;
	TSharedPtr<SGridVolumeAssetEntry> GridVolumeObjectAssetEntry;
	TArray<TSharedPtr<FGridVolumeObject>> FilterItems;
	FString meshName;
	const FString furnitureLuaTableName = TEXT("FurnitureGridGen.lua");
};