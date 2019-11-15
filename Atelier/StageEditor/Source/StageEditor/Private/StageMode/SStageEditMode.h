// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-16

#pragma once

#include "CoreMinimal.h"
#include "PlacementCategory.h"
#include "Widgets/SCompoundWidget.h"
#include "LevelAsyncLoader.h"
#include "DeclarativeSyntaxSupport.h"
#include "GCObject.h"
#include "SBorder.h"
#include "SStageModeBase.h"
#include "SStageEditorTab.h"
#include "IStructureDetailsView.h"
#include "UserStructOnScope.h"

class UWorld;
class IStageEditorInterface;
struct FAssetData;

class SStageEditMode : public SStageModeBase
{
public:
	SLATE_BEGIN_ARGS(SStageEditMode) {}
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs, IStageEditorInterface* InProxy);

	FString GetName() override;
	FString GetDescription() override;

	/** 回调 for PickLevelsDialog */
	virtual void OnPickMapAsset(const TArray<FAssetData>& SelectedAssets);
	/** 设置当前关卡的地图 */
	virtual void SetStageMap(const FString& MapPackageName);
	/** 加载关卡数据 */
	virtual void LoadStageData(const FString& StageDataFile);
	/** 清除当前关卡设置(地图 关卡数据) */
	virtual void CleanStage(bool bCleanMap, bool bCleanConfigs);
	/** 关卡可配置物件 */
	virtual void CollectPlaceableStageObject();

	void OnEditingStageChanged(FStageScopePtr Stage);

private:
	// ~Begin：Create Content Widget
	/** 顶部ToolBar */
	TSharedRef<SWidget> BuildToolbar();
	/** 编辑器主要操作部分 */
	TSharedRef<SWidget> BuildEditorMain();
	/** 基础的关卡编辑面板 */
	TSharedRef<SWidget> BuildBaseOpsPanel();
	/** 关卡天气面板 */
	TSharedRef<SWidget> BuildWeatherPanel();
	TSharedRef<SWidget> BuildExternalPanel();
	/** 可拖动的场景物件列表 */
	TSharedRef<SWidget> BuildPlaceableStageObjects();
	/***/
	void UpdateModeTools();
	// ~End: Create Content Widget

	/** 选则地图 */
	FReply OnClickSelectMap();
	/** 选则关卡配置 */
	FReply OnClickSelectStageDataFile();
	/** 清除当前关卡，包括关卡地图和地图配置 */
	FReply OnClickCleanStage();
	/** 清除关卡上的场景物件 */
	FReply OnClickCleanStageObject();
	/** 导出关卡配置 */
	FReply OnClickExportStageData();

	/** Get World */
	UWorld* GetWorld();

	FReply ExportStageConfig();
	FReply ApplyStageData();

private:
	TSharedPtr<class IStructureDetailsView> KeyPropertyView;
	/** Proxy */
	IStageEditorInterface* Proxy;
	/** 关卡地图名称 */
	FString StageMapName;
	/** 关卡配置文件 */
	FString StageDataFile;
	/** 可配置的关卡物件 */
	TArray<TSharedPtr<FPlacementCategory>> PlacementObjects;
	/** Level Loader */
	FLevelAsyncLoader LevelLoader;
	/** The container holding the mode toolbar */
	TSharedPtr< SBorder > ModeContainer;
	/** Inline content area for editor modes */
	TSharedPtr<SBorder> ModeContentHolder;
};