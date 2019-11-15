// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-24

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SUserWidget.h"
#include "Editor/EditorWidgets/Public/ITransportControl.h"

/*********************************************************/
// SSkillScrubPanel
/*********************************************************/
class SSkillScrubPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SSkillScrubPanel ) 
		: _ViewInputMin()
		, _ViewInputMax()
	{}
		SLATE_ATTRIBUTE( float, ViewInputMin )
		SLATE_ATTRIBUTE( float, ViewInputMax )
	SLATE_END_ARGS()

	/** Construct the widget */
	void Construct(const FArguments& InArgs, class SActionSkillOperatingTab* OperatorPanel);

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
	//~ End: TransportControll
private:
	/** Point to  SActionSkillOperatingTab */
	SActionSkillOperatingTab* OperatorPanel;

	/** Scrub Panel */
	TSharedPtr<class SScrubControlPanel> ScrubControlPanel;
};

/*********************************************************/
// SActionSkillOperatingTab
/*********************************************************/
class SActionSkillOperatingTab : public SCompoundWidget
{
	friend class SSkillScrubPanel;
public:
	SLATE_BEGIN_ARGS( SActionSkillOperatingTab ) {};
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<class FActionSkillEditor>& InAssetEditorToolkit);

	// ~ Begin: Skill Operation
	void AddAction();
	void DeleteAction(struct FAction& InAction);
	// ~ End: Skill Operation

	//~ Begin: Skill Preview
	void PlayActionSkill();
	void PlayActionSkillSingleAction(FAction& Action, float StartPos);
	bool IsPlayingActionSkill();
	bool IsPlayingActionSkillSingleAction(FAction& Action);
	void PauseActionSkill();
	void PauseActionSkillSingleAction();
	void SampleActionSkillAtPosition(float InPosition);
	void ForceSetActionSkillPosition(float InPosition);
	//~ End: Skill Preview
	class UActionSkillComponent* GetSkillComponent() const;

	FORCEINLINE TWeakPtr<class FActionSkillEditor> GetActionSkillEditorPtr() { return ActionSkillEditor; }
private:
	/** Construct widget for action skill */
	void ConstructActionSkill();

	/** Reconstruct */
	void ReconstructActionSkill();

	/** Get current operating skill */
	TSharedPtr<struct FActionSkillScope> GetOperatingSkill() const;

	/** On current skill changed */
	void OnOperatingSkillChanged(TWeakPtr<struct FActionSkillScope> NewSkill);

	float GetViewMinInput() const;
	float GetViewMaxInput() const;
	float GetViewStartPos(int Index) const;
	float GetScrubValue() const;
	FText GetSkillPreviewInfo() const;
	void OnChoosePrevPlaySpeed(TSharedPtr<FString> InSpeed, ESelectInfo::Type SelectInfo);
private:
	/** Weak pointer to ActionSkillEditor */
	TWeakPtr<class FActionSkillEditor> ActionSkillEditor;

	/** Widget contains ActionSegments */
	TSharedPtr<class SVerticalBox> ActionSegmentsContainer;

	/** The editors Animation Scrub Panel */
	TSharedPtr<SSkillScrubPanel> ScrubPanel;

	float CurrPlaySpeed;
	TArray<TSharedPtr<FString>> PrevPlaySpeedOptions;
};