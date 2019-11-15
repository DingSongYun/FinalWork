// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-02

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "STrackWidget.h"
#include "MultiBoxBuilder.h"
#include "AssetData.h"
#include "ActionEvent.h"
#include "DeclarativeSyntaxSupport.h"
#include "Widgets/SUserWidget.h"
#include "Editor/EditorWidgets/Public/ITransportControl.h"

struct FAction;
struct FActionKeyFrame;
class STrackWidget;

DECLARE_DELEGATE_OneParam( FOnPreActionUpdate, FAction& /*InAction*/ )
DECLARE_DELEGATE_OneParam ( FOnPostActionUpdate, FAction& /*InAction*/ )
DECLARE_DELEGATE_OneParam( FOnSkillActionClicked, FAction& /*InAction*/)

/*********************************************************/
// SActionTrack
/*********************************************************/
class SActionTrack : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SActionTrack ) : _ViewInputMin(), _ViewInputMax(), _CursorPosition(){}
	SLATE_ATTRIBUTE( float, ViewInputMin )
	SLATE_ATTRIBUTE( float, ViewInputMax )
	SLATE_ATTRIBUTE( float, CursorPosition )
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, FAction& InAction);

	virtual void ConstructNodes(TSharedPtr<STrackWidget>& Track, const FAction* InAction) {}
	/** Track color */
	virtual FLinearColor GetTrackColor() { return FLinearColor::White; }
	/** Build Right click menu */
	virtual bool BuildContextMenu(FMenuBuilder& MenuBuild) { return false; }
	/** Drop some asset to track */
	virtual void OnTrackDragDrop(TSharedPtr<class FDragDropOperation> DragDropOp, float DataPos) {}
	/** re-build track nodes */
	virtual void ReconstructNodes();

	//~ Begin: Keyborard Input
	/**  Key handler */
	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	bool SupportsKeyboardFocus() const override {return true;}
	/** Bind UI Commands */
	void BindCommands();
	/** Bind UI Commands */
	virtual void BindCommands(const class FActionTrackCommands& ActionCommands);
	//~ End: Keyborard Input
protected:
	/** Point to track widget */
	TSharedPtr<STrackWidget> 				TrackWidget;

	/** Reference to skill action */
	FAction* 								MyAction;

	TAttribute<float> 						CursorPosition;

	/** UI commands for skill panel */
	TSharedPtr<FUICommandList> 				UICommandList;
};

/*********************************************************/
// SAnimTrack
/*********************************************************/
class SAnimTrack : public SActionTrack
{
public:
	DECLARE_DELEGATE_OneParam(FOnAssignAnim, class UAnimSequenceBase* /*NewAnim*/)

	void Construct(const FArguments& InArgs, FAction& InAction, FOnAssignAnim& OnAssignAnimDelegate);
	void ConstructNodes(TSharedPtr<STrackWidget>& Track, const FAction* InAction) override;
	bool BuildContextMenu(FMenuBuilder& MenuBuild) override;
	FLinearColor GetTrackColor() override;
	void OnTrackDragDrop(TSharedPtr<class FDragDropOperation> DragDropOp, float DataPos) override;
	void ClearAnimation();
private:
	FOnAssignAnim OnAssignAnim;
};

/*********************************************************/
// SEventTrack
/*********************************************************/
class SEventTrack : public SActionTrack
{
public:
	SEventTrack();
	~SEventTrack();
	void Construct(const FArguments& InArgs, FAction& InAction, TWeakPtr<class FActionSkillEditor> ActionSkillEditorPtr);
	void ConstructNodes(TSharedPtr<STrackWidget>& Track, const FAction* InAction) override;
	FLinearColor GetTrackColor() override;
	FReply OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	bool OnBuildNodeContextMenu(FMenuBuilder& MenuBuild, FActionKeyFrame* KeyFrame);
	bool BuildContextMenu(FMenuBuilder& MenuBuild) override;
	void BindCommands(const class FActionTrackCommands& ActionCommands) override;
private:
	//~ Begin: Event base operation
	void AddNewEvent(FString NewNotifyName, UObject* NotifyStruct);
	void AddNewEvent(FString NewNotifyName, UScriptStruct* NotifyStruct, float StartTime);
	void AddNewEvent(const FActionKeyFrame& KeyFrame, float StartTime);
	void AddEventToAction(TSharedPtr<struct FActionEvent> InEvent);
	void AddEventToAction(TSharedPtr<struct FActionEvent> InEvent, float StartTime);
	void AddNewSubEvent(UScriptStruct* NotifyStruct, FActionKeyFrame* KeyFrame, FActionEventPtr NewEvent);
	void DeleteEvent(const FActionKeyFrame* KeyToDel);
	void OnCopyKeyFrameToClipboard(FActionKeyFrame* KeyFrame);
	void OnPasterKeyFrame();
	void OnDelEvent(TWeakPtr<struct FActionEvent> EventObj);
	//当Event Palette面板发生选择的时候
	void OnEditingEventChanged(TWeakPtr<struct FActionEvent> EventObj);
	//~ End: Event base operation

	void FillNewEventMenu(FMenuBuilder& MenuBuilder);
	void FillNewSubEventMenu(FMenuBuilder& MenuBuilder, FActionKeyFrame* KeyFrame);
	bool FilterActionEventStruct(const FAssetData& AssetData);
	template<typename FunctorType> void CreateEventStructPicker(FMenuBuilder& MenuBuilder, FunctorType&& OnPickEventStructFunc);
	void OnActionEventAssetSelected(const FAssetData& AssetData);
	void OnActionKeySelectionChanged(bool bIsSelected, const FActionKeyFrame* KeyFrame);
	void ChangeActionKeyTime(float NewKeyTime, FActionKeyFrame* KeyFrame);
	FName GetNodeName(const FActionKeyFrame* KeyFrame) const;
	float GetNodeStartPos(const FActionKeyFrame* KeyFrame) const;
	void SortActionKyes();
	void DeselectKeys();
	/** Input: Delete key */
	void OnDeletePressed();
	void DeleteNotifyByFrame(FActionKeyFrame* KeyFrame);
	void DeleteSelectedNodes();

private:
	TWeakPtr<class FActionSkillEditor> ActionSkillEditorPtr;

	const FActionKeyFrame* CurrSelectedKey;
	TMap<const FActionKeyFrame*, TSharedPtr<STrackNodeWidget>> KeyNodes;
	FDelegateHandle DeleteEventHandle;
	FDelegateHandle EditingEventChangedHandle;
};

/*********************************************************/
// SSkillSegmentPanel
/*********************************************************/
class SSkillSegmentPanel : public SCompoundWidget
{
	friend class SSkillSingleActionScrubPanel;
public:
	SLATE_BEGIN_ARGS( SSkillSegmentPanel ) 
		:  _OnPreActionUpdate()
		, _OnPostActionUpdate()
		, _OnSkillActionClicked()
	{}
	SLATE_ATTRIBUTE( int32, ActionIndexInSkill)
	SLATE_ATTRIBUTE( float, ActionStartPosInSkill)
	//SLATE_ATTRIBUTE( float, ScrubValue )
	SLATE_EVENT( FOnPreActionUpdate, OnPreActionUpdate )
	SLATE_EVENT( FOnPostActionUpdate, OnPostActionUpdate )
	SLATE_EVENT( FOnSkillActionClicked, OnSkillActionClicked )
	SLATE_END_ARGS()

	/** Construct the widget */
	void Construct(const FArguments& InArgs, TWeakPtr<class SActionSkillOperatingTab> HostEditor, FAction& InAction);
	/** Make Action Widget */
	void ConstructAction();
	/** Re-construct */
	void Reconstruct();

	//~ Begin: SWidget Interface
	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	//~ End: SWidget Interface

	void PlayActionSkillSingleAction();
	bool IsPlayingActionSkillSingleAction();
	void PauseActionSkillSingleAction();

	class UActionSkillComponent* GetSkillComponent() const;

	float GetViewMinInput() const;
	float GetViewMaxInput() const;
	float GetScrubValue() const;

private:
	void ProcessClickSegment();

	//~ Begin: Action Operation
	void NewAction();
	void DeleteAction();
	void AssignAnimSegment(class UAnimSequenceBase* NewSequenceBase);
	//~ End: Action Operation
private:

	/** Point to skill operator editor */
	TWeakPtr<class SActionSkillOperatingTab>	OptEditorPtr;

	/** The editors Animation Scrub Panel */
	TSharedPtr<SSkillSingleActionScrubPanel> ScrubPanel;

	/** Panel Root */
	TSharedPtr<SBorder> 						PanelArea;

	/** Skill Action Segment */
	FAction* 									ActionPtr;

	TAttribute<int32>                           ActionIndex;
	TAttribute<float>                           ActionStartPos;
	TAttribute<float>							ScrubValue;
	/** Events */
	FOnPreActionUpdate 							OnPreActionUpdate;
	FOnPostActionUpdate 						OnPostActionUpdate;
	FOnSkillActionClicked 						OnSkillActionClicked;
};

/*********************************************************/
// SSkillSingleActionScrubPanel
/*********************************************************/
class SSkillSingleActionScrubPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSkillSingleActionScrubPanel)
		: _ViewInputMin()
		, _ViewInputMax()
	{}
	SLATE_ATTRIBUTE(float, ViewInputMin)
	SLATE_ATTRIBUTE(float, ViewInputMax)
	SLATE_END_ARGS()

	/** Construct the widget */
	void Construct(const FArguments& InArgs, class SSkillSegmentPanel* SegmentPanel);

	float GetScrubValue() const;
private:
	uint32 GetNumOfFrames() const;
	float GetSequenceLength() const;
	bool IsRealtimeStreamingMode() const;

	void OnScrubValueChanged(float NewValue);

	//~ Begin: TransportControll
	virtual FReply OnClick_Forward_Step();
	virtual FReply OnClick_Forward_End();
	virtual FReply OnClick_Backward_Step();
	virtual FReply OnClick_Backward_End();
	virtual FReply OnClick_Forward();
	virtual FReply OnClick_Backward();
	virtual FReply OnClick_ToggleLoop();
	virtual FReply OnClick_Record();
	EPlaybackMode::Type GetPlaybackMode() const;
	bool IsRecording() const;
	bool IsLoopStatusOn() const;

	TAttribute<float>                           ViewMinInput;
	TAttribute<float>							ViewMaxInput;
	//~ End: TransportControll
private:
	/** Point to  SActionSkillOperatingTab */
	SSkillSegmentPanel* SegmentPanel;

	/** Scrub Panel */
	TSharedPtr<class SScrubControlPanel> ScrubControlPanel;
};