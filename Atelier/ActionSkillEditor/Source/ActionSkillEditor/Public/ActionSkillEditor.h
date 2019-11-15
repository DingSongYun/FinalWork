// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-19

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "EditorUndoClient.h"
#include "TickableEditorObject.h"
#include "IAssetFamily.h"
#include "WorkflowCentricApplication.h"

struct FActionSkillScope;
struct FActionEvent;
struct FActionKeyFrame;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEditingSkillChangedDelegate, TWeakPtr<FActionSkillScope>/*NewSkill*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEditingEventChangedDelegate, TWeakPtr<FActionEvent>/*NewEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEditingKeyFrameSetDelegate, FActionKeyFrame*/*NewActionKey*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnNewSkillDelegate, TWeakPtr<FActionSkillScope> /*NewSkill*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeleteSkillDelegate, TWeakPtr<FActionSkillScope>/*DeletedSkill*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnNewEventDelegate, TWeakPtr<FActionEvent> /*NewEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeleteEventDelegate, TWeakPtr<FActionEvent>/*DeletedEvent*/);

class UActionSkillTable;
class UActionEventTable;

namespace ActionSkillEditorModes
{
	// Mode identifiers
	extern const FName SkillEditorMode;
	extern const FName SkillDeclareMode;
}

namespace ActionSkillEditorTabs
{
	// Skill Edit Tabs
	extern const FName SkillPalletteTab;
	extern const FName EventPalletteTab;
	extern const FName ViewportTab;
	extern const FName OperatingTab;
	extern const FName ActionDetailTab;
	extern const FName EventDetailTab;
	extern const FName PrevSceneSettingTab;
	extern const FName WorldOutlineTab;
	extern const FName AssetBrowserTab;

	// Struct Define Tabs
	extern const FName SkillStructTab;
	extern const FName SubEventStructTab;
	extern const FName ActionEventStructsTab;
}

class IActionSkillEditor : public FWorkflowCentricApplication
{
public:
	virtual void CreatePreviewScene() = 0;
	virtual const UActionSkillTable* GetSkillTable() const = 0;
	virtual const UActionEventTable* GetEventTable() const = 0;
	virtual TSharedPtr<FActionSkillScope>& GetEditingSkill() = 0;
	virtual TSharedPtr<FActionEvent>& GetEditingEvent() = 0;
	virtual TSharedPtr<FActionEvent>& GetEditingEventInKeyFrame() = 0;
	virtual void SetEditingSkill(TSharedPtr<FActionSkillScope> InActionSkill) = 0;
	virtual void SetEditingEvent(TSharedPtr<FActionEvent> InActionEvent) = 0;
	virtual void SetEditingActionKeyFrame(FActionKeyFrame* InActionKeyFrame) = 0;
	virtual FActionKeyFrame* GetEditingActionKeyFrame() = 0;
	virtual class ISkillPreviewProxy* GetSkillPreviewProxy() = 0;

	//~ Begin: Event Delegates
	virtual FOnEditingSkillChangedDelegate& OnEditingSkillChanged() = 0;
	virtual FOnEditingEventChangedDelegate& OnEditingEventChanged() = 0;
	virtual FOnNewSkillDelegate& OnNewSkill() = 0;
	virtual FOnDeleteSkillDelegate& OnDeleteSkill() = 0;
	virtual FOnNewEventDelegate& OnNewEvent() = 0;
	virtual FOnDeleteEventDelegate& OnDeleteEvent() = 0;
	virtual FOnEditingKeyFrameSetDelegate& OnEditingKeyFrameSet() = 0;
	//~ End: Event Delegates
};

class ACTIONSKILLEDITOR_API FActionSkillEditor : public IActionSkillEditor, public FGCObject, public FEditorUndoClient, public FTickableEditorObject
{
	friend class FActionSkillEditorMode;
	friend class FActionSkillDefineMode;
public:
	FActionSkillEditor();
	virtual ~FActionSkillEditor();

	void InitializeEditor();

	//~ Begin: IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual bool OnRequestClose() override;
	//~ End: IToolkit interface


	//~ Begin: FAssetEditorToolkit interface
	/** Called when "Save" is clicked for this asset */
	virtual void SaveAsset_Execute();

	/** We disable SaveAs here */
	virtual bool CanSaveAssetAs() const {return false;}
	//~ End: FAssetEditorToolkit interface

	//~ Begin: IActionSkillEditor interface
	virtual void CreatePreviewScene() override;
	virtual const UActionSkillTable* GetSkillTable() const override;
	virtual const UActionEventTable* GetEventTable() const override;
	FORCEINLINE TSharedPtr<FActionSkillScope>& GetEditingSkill() override { return EditingActionSkill; }
	FORCEINLINE TSharedPtr<FActionEvent>& GetEditingEvent() override { return EditingActionEvent; }
	FORCEINLINE TSharedPtr<FActionEvent>& GetEditingEventInKeyFrame() override { return EditingActionEventKeyFrame; }
	void SetEditingSkill(TSharedPtr<FActionSkillScope> InActionSkill) override;
	void SetEditingEvent(TSharedPtr<FActionEvent> InActionEvent) override;
	void SetEditingActionKeyFrame(FActionKeyFrame* InActionKeyFrame) override;
	FORCEINLINE FActionKeyFrame* GetEditingActionKeyFrame() override { return EditingActionKeyFrame; }
	virtual class ISkillPreviewProxy* GetSkillPreviewProxy() override;
	FORCEINLINE FOnEditingSkillChangedDelegate& OnEditingSkillChanged() override { return OnEditingSkillChangedEvent; }
	FORCEINLINE FOnEditingEventChangedDelegate& OnEditingEventChanged() override { return OnEditingEventChangedEvent; }
	FORCEINLINE FOnEditingKeyFrameSetDelegate& OnEditingKeyFrameSet() override { return OnEditingKeyFrameSetEvent; }
	FORCEINLINE FOnNewSkillDelegate& OnNewSkill() override { return OnNewSkillEvent; }
	FORCEINLINE FOnDeleteSkillDelegate& OnDeleteSkill() override { return OnDeleteSkillEvent; }
	FORCEINLINE FOnNewEventDelegate& OnNewEvent() override { return OnNewEventEvent; }
	FORCEINLINE FOnDeleteEventDelegate& OnDeleteEvent() override { return OnDeleteEventEvent; }
	//~ End: IActionSkillEditor interface

	TSharedPtr<class FActionSkillEditorPreviewScene> GetPrviewScene() { return PreviewScene; }
	class ACharacter* GetPreviewSkillCaster();

	//~ Begin: FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End: FGCObject interface

	//~ Begin: FEditorUndoClient interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	//~ End: FEditorUndoClient interface

	//~ Begin: FTickableEditorObject interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	//~ End: FTickableEditorObject interface

	/** 
	 * When mode changed from editor mode to skill struct declared mode
	 * we need clean some data and delegate here
	 */
	void OnPreEditorModeDeactivated();

	// Create A Action Skill
	TSharedPtr<FActionSkillScope> NewAndEditSkill();
	// Delete Skill

	TSharedPtr<FActionEvent> NewAndEditEvent();

	void NewEventStructureType();

	void DeleteAndEditSkill(TSharedPtr<FActionSkillScope> SkillToDel);

	void DeleteAndEditEvent(TSharedPtr<FActionEvent> EventToDel);

private:
	void LoadSkillTable();

	void ExtendToolbar();

	TSharedRef<SDockTab> SpawnTab_Palette(const FSpawnTabArgs& Args);

	// TSharedRef<IAssetFamily> CreateSkillAssetFamily();
	void ExtendModeToolbar(class FToolBarBuilder& InToolbarBuilder);

	void OnClickCreateSkill() { NewAndEditSkill(); }
	void OnClickNewEventType () { NewEventStructureType(); }

private:
	/** Multicast delegate fired on global undo/redo */
	FSimpleMulticastDelegate 							OnPostUndo;

	/** PreviewScene for editor */
	TSharedPtr<class FActionSkillEditorPreviewScene> 	PreviewScene;

	TSharedPtr<FActionSkillScope> 						EditingActionSkill;
	TSharedPtr<FActionEvent> 							EditingActionEvent;
	TSharedPtr<FActionEvent> 							EditingActionEventKeyFrame;
	FActionKeyFrame*				 					EditingActionKeyFrame;

	/** Toolbar extender */
	TSharedPtr<FExtender> 								ToolbarExtender;

	/** Palette of Material Expressions and functions */
	TSharedPtr<class SActionSkillPalette> 				Palette;

	//~ Begin: Events
	FOnEditingSkillChangedDelegate 						OnEditingSkillChangedEvent;
	FOnEditingEventChangedDelegate 						OnEditingEventChangedEvent;
	FOnEditingKeyFrameSetDelegate						OnEditingKeyFrameSetEvent;
	FOnNewSkillDelegate 								OnNewSkillEvent;
	FOnDeleteSkillDelegate 								OnDeleteSkillEvent;
	FOnNewEventDelegate 								OnNewEventEvent;
	FOnDeleteEventDelegate 								OnDeleteEventEvent;
	//~ End: Events
};
