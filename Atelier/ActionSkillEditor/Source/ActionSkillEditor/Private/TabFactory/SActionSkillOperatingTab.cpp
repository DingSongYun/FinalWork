// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-24

#include "SActionSkillOperatingTab.h"
#include "UnrealEd.h"
#include "UserWidget.h"
#include "ActionSkill.h"
#include "ActionSkillEditor.h"
#include "SScrubControlPanel.h"
#include "SSkillSegmentPanel.h"
#include "EditorWidgetsModule.h"
#include "GameFramework/Character.h"
#include "ActionSkillComponent.h"
#include "ISkillPreviewProxy.h"
#include "Widgets/Input/STextComboBox.h"

#define LOCTEXT_NAMESPACE "SActionSkillOperatingTab"

/*********************************************************/
// SSkillScrubPanel
/*********************************************************/
void SSkillScrubPanel::Construct(const FArguments& InArgs, SActionSkillOperatingTab* InOperatorPanel)
{
	OperatorPanel = InOperatorPanel;

	this->ChildSlot
	[
		SNew(SHorizontalBox)
		.AddMetaData<FTagMetaData>(TEXT("AnimScrub.Scrub"))
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.FillWidth(1)
		.Padding(0.0f)
		[
			SAssignNew(ScrubControlPanel, SScrubControlPanel)
			.IsEnabled(true)
			.Value(this, &SSkillScrubPanel::GetScrubValue)
			.NumOfKeys(this, &SSkillScrubPanel::GetNumOfFrames)
			.SequenceLength(this, &SSkillScrubPanel::GetSequenceLength)
			.OnValueChanged(this, &SSkillScrubPanel::OnScrubValueChanged)
			.ViewInputMin(InArgs._ViewInputMin)
			.ViewInputMax(InArgs._ViewInputMax)
			.DisplayDrag(true)
			.bAllowZoom(true)
			.IsRealtimeStreamingMode(this, 	&SSkillScrubPanel::IsRealtimeStreamingMode)
		]
	];
}

float SSkillScrubPanel::GetScrubValue() const
{
	if (auto SkillComp =  OperatorPanel->GetSkillComponent())
	{
		return SkillComp->GetPosition();
	}
	return 0;
}

void SSkillScrubPanel::OnScrubValueChanged(float NewValue)
{
	OperatorPanel->SampleActionSkillAtPosition(NewValue);
}

uint32 SSkillScrubPanel::GetNumOfFrames() const
{
	auto Skill = OperatorPanel->GetOperatingSkill();
	if (Skill.IsValid())
	{
		return Skill->CalculateSkillFrame();
	}
	return 30;
}

float SSkillScrubPanel::GetSequenceLength() const
{
	auto Skill = OperatorPanel->GetOperatingSkill();
	if (Skill.IsValid())
	{
		return Skill->CalculateSkillLength();
	}

	return 30.f;
}

bool SSkillScrubPanel::IsRealtimeStreamingMode() const
{
	return false;
}

FReply SSkillScrubPanel::OnClick_Forward_Step()
{
	return FReply::Handled();
}

FReply SSkillScrubPanel::OnClick_Forward_End()
{
	return FReply::Handled();
}

FReply SSkillScrubPanel::OnClick_Backward_Step()
{
	return FReply::Handled();
}

FReply SSkillScrubPanel::OnClick_Backward_End()
{
	return FReply::Handled();
}

FReply SSkillScrubPanel::OnClick_Forward()
{
	return FReply::Handled();
}

FReply SSkillScrubPanel::OnClick_Backward()
{
	return FReply::Handled();
}

FReply SSkillScrubPanel::OnClick_ToggleLoop()
{
	return FReply::Handled();
}

FReply SSkillScrubPanel::OnClick_Record()
{
	return FReply::Handled();
}

EPlaybackMode::Type SSkillScrubPanel::GetPlaybackMode() const
{
	return EPlaybackMode::PlayingForward;
}

bool SSkillScrubPanel::IsRecording() const
{
	return false;
}

bool SSkillScrubPanel::IsLoopStatusOn() const
{
	return false;
}

struct FActionSkillControl
{
public:
	static FReply OnForwardPlayCilck(SActionSkillOperatingTab* Operator)
	{
		bool bIsPlaying = Operator->IsPlayingActionSkill();
		if (bIsPlaying)
		{
			Operator->PauseActionSkill();
		}
		else
		{
			Operator->PlayActionSkill();
		}
		return FReply::Handled();
	}

	static EPlaybackMode::Type GetPlaybackMode(SActionSkillOperatingTab* Operator)
	{
		bool bIsPlaying = Operator->IsPlayingActionSkill();
		return bIsPlaying ? EPlaybackMode::PlayingForward : EPlaybackMode::Stopped;
	}

	static FReply OnForwardStepCilck(SActionSkillOperatingTab* Operator)
	{
		return FReply::Handled();
	}

	static FReply OnForwardEndCilck(SActionSkillOperatingTab* Operator)
	{
		return FReply::Handled();
	}
};

/*********************************************************/
// SActionSkillOperatingTab
/*********************************************************/
void SActionSkillOperatingTab::Construct(const FArguments& InArgs, const TSharedRef<FActionSkillEditor>& InAssetEditorToolkit)
{
	ActionSkillEditor = InAssetEditorToolkit;

	// For play controller
	FEditorWidgetsModule& EditorWidgetsModule = FModuleManager::Get().LoadModuleChecked<FEditorWidgetsModule>( "EditorWidgets" );
	FTransportControlArgs TransportControlArgs;
	{
		TransportControlArgs.OnForwardPlay = FOnClicked::CreateStatic(&FActionSkillControl::OnForwardPlayCilck, this);
		TransportControlArgs.OnGetPlaybackMode = FOnGetPlaybackMode::CreateStatic(&FActionSkillControl::GetPlaybackMode, this);
		// TransportControlArgs.OnForwardStep = FOnClicked::CreateStatic(&FActionSkillControl::OnForwardStepCilck, this);
		// TransportControlArgs.OnForwardEnd = FOnClicked::CreateStatic(&FActionSkillControl::OnForwardEndCilck, this);
	}

	PrevPlaySpeedOptions.Add(MakeShareable(new FString("0.25")));
	PrevPlaySpeedOptions.Add(MakeShareable(new FString("0.5")));
	PrevPlaySpeedOptions.Add(MakeShareable(new FString("1")));
	PrevPlaySpeedOptions.Add(MakeShareable(new FString("2")));
	PrevPlaySpeedOptions.Add(MakeShareable(new FString("5")));
	CurrPlaySpeed = 1;

	this->ChildSlot
	[
		SNew(SVerticalBox)

		//+SVerticalBox::Slot()
		//.AutoHeight()
		//.VAlign(VAlign_Top)
		//[
		//	SAssignNew( ScrubPanel, SSkillScrubPanel, this)
		//	.ViewInputMin(this, &SActionSkillOperatingTab::GetViewMinInput)
		//	.ViewInputMax(this, &SActionSkillOperatingTab::GetViewMaxInput)
		//]

		+SVerticalBox::Slot()
		.FillHeight(1)
		.Padding(FMargin(0, 15.f, 0, 0))
		[
			SAssignNew(ActionSegmentsContainer, SVerticalBox)
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1)
			.Padding(10, 0, 0, 0)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(this, &SActionSkillOperatingTab::GetSkillPreviewInfo)
				]
			]

			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			.Padding(10, 0, 0, 0)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(LOCTEXT("ActionSKill_Prev_PlaySpeed", "Speed"))
				]

				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(5, 10, 0, 10)
				[
					SNew(STextComboBox)
					.OptionsSource(&PrevPlaySpeedOptions)
					.OnSelectionChanged(this, &SActionSkillOperatingTab::OnChoosePrevPlaySpeed)
					.InitiallySelectedItem(PrevPlaySpeedOptions[2])
				]
			]

			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(10, 0, 0, 0)
			[
				EditorWidgetsModule.CreateTransportControl(TransportControlArgs)
			]
		]
	];

	ConstructActionSkill();
	ActionSkillEditor.Pin()->OnEditingSkillChanged().AddRaw(this, &SActionSkillOperatingTab::OnOperatingSkillChanged);
}

void SActionSkillOperatingTab::ConstructActionSkill()
{
	auto ActionSkill = GetOperatingSkill();

	if (!ActionSkill.IsValid()) return;

	int32 ActionIndex = 0;
	float ActionStartPos = 0;
	for (auto It = ActionSkill->CreateActionIterator(); It; ++It)
	{
		FAction& Action = *It;
		ActionSegmentsContainer->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0, 0, 0, 10))
		[
			SNew(SBorder)
			.Padding(FMargin(5, 10, 5, 10))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SSkillSegmentPanel, SharedThis(this), Action)
				.ActionIndexInSkill(ActionIndex)
				.ActionStartPosInSkill(this, &SActionSkillOperatingTab::GetViewStartPos, ActionIndex)
			]
		];
		ActionIndex++;
		ActionStartPos += Action.GetActionLength();
	}
}

void SActionSkillOperatingTab::ReconstructActionSkill()
{
	if (!ActionSegmentsContainer.IsValid()) return;

	ActionSegmentsContainer->ClearChildren();
	ConstructActionSkill();
}

void SActionSkillOperatingTab::OnOperatingSkillChanged(TWeakPtr<struct FActionSkillScope> NewSkill)
{
	ReconstructActionSkill();
}

void SActionSkillOperatingTab::PlayActionSkill()
{
	TSharedPtr<FActionSkillScope> Skill = GetOperatingSkill();
	if (Skill.IsValid())
	{
		if (auto PreviewProxy = ActionSkillEditor.Pin()->GetSkillPreviewProxy())
		{
			PreviewProxy->PlayActionSkill(Skill->GetId());
			PreviewProxy->SetPlayRate(CurrPlaySpeed);
		}
	}
}

void SActionSkillOperatingTab::PlayActionSkillSingleAction(FAction& Action, float StartPos)
{
	TSharedPtr<FActionSkillScope> Skill = GetOperatingSkill();
	if (Skill.IsValid())
	{
		if (auto PreviewProxy = ActionSkillEditor.Pin()->GetSkillPreviewProxy())
		{
			PreviewProxy->PlayActionSkillSingleAction(Skill->GetId(), Action, StartPos);
			PreviewProxy->SetPlayRate(CurrPlaySpeed);
		}
	}
}

bool SActionSkillOperatingTab::IsPlayingActionSkill()
{
	if (auto PreviewProxy = ActionSkillEditor.Pin()->GetSkillPreviewProxy())
	{
		return PreviewProxy->IsPlayingActionSkill();
	}
	return false;
}

bool SActionSkillOperatingTab::IsPlayingActionSkillSingleAction(FAction& Action)
{
	if (auto PreviewProxy = ActionSkillEditor.Pin()->GetSkillPreviewProxy())
	{
		return PreviewProxy->IsPlayingActionSkillSingleAction(Action);
	}
	return false;
}

void SActionSkillOperatingTab::PauseActionSkill()
{
	if (auto PreviewProxy = ActionSkillEditor.Pin()->GetSkillPreviewProxy())
	{
		PreviewProxy->PauseActionSkill();
	}
}

void SActionSkillOperatingTab::PauseActionSkillSingleAction()
{
	if (auto PreviewProxy = ActionSkillEditor.Pin()->GetSkillPreviewProxy())
	{
		PreviewProxy->PauseActionSkillSingleAction();
	}
}

void SActionSkillOperatingTab::SampleActionSkillAtPosition(float InPosition)
{
	auto Skill = GetOperatingSkill();
	if (Skill.IsValid())
	{
		if (auto PreviewProxy = ActionSkillEditor.Pin()->GetSkillPreviewProxy())
		{
			PreviewProxy->SampleActionSkillAtPosition(Skill->GetId(), InPosition);
		}
	}
}
void SActionSkillOperatingTab::ForceSetActionSkillPosition(float InPosition)
{
	auto Skill = GetOperatingSkill();
	if (Skill.IsValid())
	{
		if (auto PreviewProxy = ActionSkillEditor.Pin()->GetSkillPreviewProxy())
		{
			PreviewProxy->SampleActionSkillAtPosition(Skill->GetId(), InPosition);
			PreviewProxy->PlayActionSkill(Skill->GetId());
			PreviewProxy->PauseActionSkill();
		}
	}
}

class UActionSkillComponent* SActionSkillOperatingTab::GetSkillComponent() const
{
	if (ACharacter* Caster = ActionSkillEditor.Pin()->GetPreviewSkillCaster())
	{
		return Cast<UActionSkillComponent>(Caster->GetComponentByClass(UActionSkillComponent::StaticClass()));
	}

	return nullptr;
}

float SActionSkillOperatingTab::GetViewStartPos(int Index) const 
{
	auto Skill = GetOperatingSkill();
	if (Skill.IsValid())
	{
		return Skill->GetSkillActionStartPos(Index);
	}
	return 0;
}

TSharedPtr<FActionSkillScope> SActionSkillOperatingTab::GetOperatingSkill() const
{
	return ActionSkillEditor.Pin()->GetEditingSkill();
}

float SActionSkillOperatingTab::GetViewMinInput() const
{
	return 0.f;
}

float SActionSkillOperatingTab::GetViewMaxInput() const
{
	auto Skill = GetOperatingSkill();
	if (Skill.IsValid())
	{
		return Skill->CalculateSkillLength();
	}

	return 60.f;
}

float SActionSkillOperatingTab::GetScrubValue() const
{
	if (auto SkillComp = GetSkillComponent())
	{
		return SkillComp->GetPosition();
	}

	return 0.f;
}

FText SActionSkillOperatingTab::GetSkillPreviewInfo() const
{
	float PlayPercentage = GetViewMaxInput() > 0 ? GetScrubValue() / GetViewMaxInput() : 0;
	return FText::FromString(FString::Printf(TEXT("Percentage: %f%, CurrentTime: %f / %f (s)"), 
			PlayPercentage,
			GetScrubValue(),
			GetViewMaxInput()
		));
}

void SActionSkillOperatingTab::OnChoosePrevPlaySpeed(TSharedPtr<FString> InSpeed, ESelectInfo::Type SelectInfo)
{
	float Speed = FCString::Atof(**InSpeed);
	GetSkillComponent()->SetPlayRate(Speed);
	CurrPlaySpeed = Speed;
}

void SActionSkillOperatingTab::AddAction()
{
	TSharedPtr<struct FActionSkillScope> Skill = GetOperatingSkill();
	if (Skill.IsValid())
	{
		Skill->NewAction();
		Skill->MarkDirty(true);

		ReconstructActionSkill();
	}
}

void SActionSkillOperatingTab::DeleteAction(FAction& InAction)
{
	TSharedPtr<struct FActionSkillScope> Skill = GetOperatingSkill();
	if (Skill.IsValid())
	{
#if 0
			/* TODO:
			 * 很有意思的是这里用Remove会报错，
			 * Array在Remove的时候会检查Item的地址的合法性(是不是在Array地址快内)
			 * 没太仔细看地址检查失败的原因，也不太理解Array在这里的操作，毕竟Array的地址也不应该是连续的, 
			 * 不连续的话地址的检查意义何在
			 */
			Skill->Actions.Remove(InAction);
#else
	/* 
		TArray<FAction> Actions = Skill->GetActions();
		int32 Index = Actions.Find(InAction);
		if (Index != INDEX_NONE)
		{
			Actions.RemoveAt(Index);
			Skill->MarkDirty(true);
			ReconstructActionSkill();
		}
	*/
#endif
		Skill->RemoveAction(InAction);
		Skill->MarkDirty(true);
		ReconstructActionSkill();
	}
}

#undef LOCTEXT_NAMESPACE