// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-21
#include "ActionSkillTabSpawners.h"
#include "ActionSkillEditor.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"
#include "SActionSkillPalette.h"
#include "SActionEventPalette.h"
#include "SActionSkillViewport.h"
#include "SActionSkillOperatingTab.h"
#include "SActionSkillDetailTab.h"
#include "SActionKeyDetailTab.h"
#include "SSkillAssetBrowserTab.h"
#include "AdvancedPreviewSceneModule.h"
#include "Modules/ModuleManager.h"
#include "ActionSkillEditorPreviewScene.h"
#include "SceneOutlinerPublicTypes.h"
#include "SceneOutlinerModule.h"
#include "Widgets/Docking/SDockTab.h"

/*********************************************************************/
// FSkillPalletteTabSummoner
// 技能清单面板
/*********************************************************************/
struct FSkillPalletteTabSummoner : public FWorkflowTabFactory
{
public:
	static TSharedRef<class FWorkflowTabFactory> Create(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp) 
	{
		return MakeShareable(new FSkillPalletteTabSummoner(InHostingApp));
	}
public:
	FSkillPalletteTabSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
		: FWorkflowTabFactory(ActionSkillEditorTabs::SkillPalletteTab, InHostingApp)
	{
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
	{
		TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingApp.Pin());
		return SNew(SActionSkillPalette, ActionSkillEditorPtr)
			.OnSkillSelected(ActionSkillEditorPtr.Get(), &FActionSkillEditor::SetEditingSkill)
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("SkillPallette")));
	}
};

/*********************************************************************/
// FEventPalletteTabSummoner
// 技能事件清单面板
/*********************************************************************/
struct FEventPalletteTabSummoner : public FWorkflowTabFactory
{
public:
	static TSharedRef<class FWorkflowTabFactory> Create(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp) 
	{
		return MakeShareable(new FEventPalletteTabSummoner(InHostingApp));
	}
public:
	FEventPalletteTabSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
		: FWorkflowTabFactory(ActionSkillEditorTabs::EventPalletteTab, InHostingApp)
	{
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
	{
		TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingApp.Pin());
		return SNew(SActionEventPalette, ActionSkillEditorPtr)
			.OnEventSelected(ActionSkillEditorPtr.Get(), &FActionSkillEditor::SetEditingEvent)
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ActionPallette")));
	}
};

/*********************************************************************/
// FPreviewViewportSummoner
// 技能预览
/*********************************************************************/
struct FPreviewViewportSummoner: public FWorkflowTabFactory
{
public:
	static TSharedRef<class FWorkflowTabFactory> Create(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp) 
	{
		return MakeShareable(new FPreviewViewportSummoner(InHostingApp));
	}
public:
	FPreviewViewportSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
		: FWorkflowTabFactory(ActionSkillEditorTabs::ViewportTab, InHostingApp)
	{
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
	{
		TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingApp.Pin());
		return SNew(SActionSkillEdViewportTab, ActionSkillEditorPtr->GetPrviewScene().ToSharedRef(), ActionSkillEditorPtr.ToSharedRef(), 0)
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ActionSkillViewport")));
	}
};

/*********************************************************************/
// FSkillOperatingTabSummoner
// 技能编辑
/*********************************************************************/
struct FSkillOperatingTabSummoner: public FWorkflowTabFactory
{
public:
	static TSharedRef<class FWorkflowTabFactory> Create(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp) 
	{
		return MakeShareable(new FSkillOperatingTabSummoner(InHostingApp));
	}
public:
	FSkillOperatingTabSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
		: FWorkflowTabFactory(ActionSkillEditorTabs::OperatingTab, InHostingApp)
	{
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
	{
		TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingApp.Pin());
		return SNew(SActionSkillOperatingTab, ActionSkillEditorPtr.ToSharedRef())
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ActionSkillOperatingTab")));
	}
};

/*********************************************************************/
// FActionDetailTabSummoner
// 技能详情
/*********************************************************************/
struct FActionDetailTabSummoner: public FWorkflowTabFactory
{
public:
	static TSharedRef<class FWorkflowTabFactory> Create(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp) 
	{
		return MakeShareable(new FActionDetailTabSummoner(InHostingApp));
	}
public:
	FActionDetailTabSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
		: FWorkflowTabFactory(ActionSkillEditorTabs::ActionDetailTab, InHostingApp)
	{
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
	{
		TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingApp.Pin());
		return SNew(SActionSkillDetailTab, ActionSkillEditorPtr)
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ActionSkillDetailTab")));
	}
};

/*********************************************************************/
// FEventDetailTabSummoner
// 事件帧编辑
/*********************************************************************/
struct FEventDetailTabSummoner: public FWorkflowTabFactory
{
public:
	static TSharedRef<class FWorkflowTabFactory> Create(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp) 
	{
		return MakeShareable(new FEventDetailTabSummoner(InHostingApp));
	}
public:
	FEventDetailTabSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
		: FWorkflowTabFactory(ActionSkillEditorTabs::EventDetailTab, InHostingApp)
	{
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
	{
		TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingApp.Pin());
		return SNew(SActionKeyDetailTab, ActionSkillEditorPtr)
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("EventDetailTab")));
	}
};

/*********************************************************************/
// FPrevSceneSettingTabSummoner
// 技能详情
/*********************************************************************/
struct FPrevSceneSettingTabSummoner: public FWorkflowTabFactory
{
public:
	static TSharedRef<class FWorkflowTabFactory> Create(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp) 
	{
		return MakeShareable(new FPrevSceneSettingTabSummoner(InHostingApp));
	}
public:
	FPrevSceneSettingTabSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
		: FWorkflowTabFactory(ActionSkillEditorTabs::PrevSceneSettingTab, InHostingApp)
	{
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
	{
		TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingApp.Pin());
		TSharedRef<FActionSkillEditorPreviewScene> PrevSceneRef = ActionSkillEditorPtr->GetPrviewScene().ToSharedRef();

		TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo> DetailsCustomizations;
		TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo> PropertyTypeCustomizations;
		PrevSceneRef->OnPreviewSceneSettingCustomization(DetailsCustomizations, PropertyTypeCustomizations);

		// PrevSceneRef->
		FAdvancedPreviewSceneModule& AdvancedPreviewSceneModule = FModuleManager::LoadModuleChecked<FAdvancedPreviewSceneModule>("AdvancedPreviewScene");
		return AdvancedPreviewSceneModule.CreateAdvancedPreviewSceneSettingsWidget(PrevSceneRef, 
				PrevSceneRef->GetPreviewSceneAddSetting(), DetailsCustomizations, PropertyTypeCustomizations);
	}
};

/*********************************************************************/
// FAction
// WorldOutline
/*********************************************************************/
struct FSkillEditorWorldOutlineTabSummoner: public FWorkflowTabFactory
{
public:
	static TSharedRef<class FWorkflowTabFactory> Create(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp) 
	{
		return MakeShareable(new FSkillEditorWorldOutlineTabSummoner(InHostingApp));
	}
public:
	FSkillEditorWorldOutlineTabSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
		: FWorkflowTabFactory(ActionSkillEditorTabs::WorldOutlineTab, InHostingApp)
	{
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
	{
		TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingApp.Pin());
		TSharedRef<FActionSkillEditorPreviewScene> PrevSceneRef = ActionSkillEditorPtr->GetPrviewScene().ToSharedRef();

		SceneOutliner::FInitializationOptions InitOptions;
		InitOptions.Mode = ESceneOutlinerMode::ActorBrowsing;
		InitOptions.SpecifiedWorldToDisplay = PrevSceneRef->GetWorld();
		FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::Get().LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");
		TSharedRef<ISceneOutliner> SceneOutlinerRef = SceneOutlinerModule.CreateSceneOutliner(
			InitOptions,
			FOnActorPicked() /* Not used for outliner when in browsing mode */);
		return SNew(SBorder)
				.Padding(4)
				[
					SceneOutlinerRef
				];
	}
};

/*********************************************************************/
// FAction
// WorldOutline
/*********************************************************************/
struct FSkillAssetBrowserTabSummoner: public FWorkflowTabFactory
{
public:
	static TSharedRef<class FWorkflowTabFactory> Create(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp) 
	{
		return MakeShareable(new FSkillAssetBrowserTabSummoner(InHostingApp));
	}
public:
	FSkillAssetBrowserTabSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
		: FWorkflowTabFactory(ActionSkillEditorTabs::AssetBrowserTab, InHostingApp)
	{
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
	{
		TSharedPtr<FActionSkillEditor> ActionSkillEditorPtr = StaticCastSharedPtr<FActionSkillEditor>(HostingApp.Pin());
		return SNew(SSkillAssetBrowserTab, ActionSkillEditorPtr)
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("SkillAssetBrowserTab")));
	}
};

namespace TabSpawners
{
	void RegisterActionSkillTabs(const TSharedRef<class FWorkflowCentricApplication> HostingAppPtr, class FWorkflowAllowedTabSet& TabFactories)
	{
		TabFactories.RegisterFactory(FSkillPalletteTabSummoner::Create(HostingAppPtr));
		TabFactories.RegisterFactory(FEventPalletteTabSummoner::Create(HostingAppPtr));
		TabFactories.RegisterFactory(FPreviewViewportSummoner::Create(HostingAppPtr));
		TabFactories.RegisterFactory(FSkillOperatingTabSummoner::Create(HostingAppPtr));
		TabFactories.RegisterFactory(FActionDetailTabSummoner::Create(HostingAppPtr));
		TabFactories.RegisterFactory(FEventDetailTabSummoner::Create(HostingAppPtr));
		TabFactories.RegisterFactory(FPrevSceneSettingTabSummoner::Create(HostingAppPtr));
		TabFactories.RegisterFactory(FSkillEditorWorldOutlineTabSummoner::Create(HostingAppPtr));
		TabFactories.RegisterFactory(FSkillAssetBrowserTabSummoner::Create(HostingAppPtr));
	}
}