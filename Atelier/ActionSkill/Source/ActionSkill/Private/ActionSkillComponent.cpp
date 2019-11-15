// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-16
#include "ActionSkillComponent.h"
#include "GameFramework/Character.h"
#include "Modules/ModuleManager.h"
#include "ActionSkill.h"

FSkillActionStepper::FSkillActionStepper(UActionSkillInstance* SkillPlayer)
    : Player(SkillPlayer), LastStepTime(0)
{
}

void FSkillActionStepper::InitAction(FAction& InAction)
{
    Action = &InAction;
    LastStepTime = 0;
}

void FSkillActionStepper::Start()
{
#if 0
    Player->GetSkillComponent()->PlayAnimation(Action->AnimName.ToString());
#else
    if (!Action->AnimReference.IsNull())
    {
        auto SkilLComp = Player->GetSkillComponent();
        SkilLComp->PlayAnimation(GetActionAnimAsset(), SkilLComp->GetPlayRate());
    }
#endif
}

UAnimSequenceBase* FSkillActionStepper::GetActionAnimAsset()
{
    if (!Action->AnimReference.IsNull())
    {
        return (Action->AnimReference ? 
                    &*Action->AnimReference : Action->AnimReference.LoadSynchronous());
    }

    return nullptr;
}

void FSkillActionStepper::Advance(float DeltaTime)
{
    StepActionTo(LastStepTime, LastStepTime + DeltaTime);
    LastStepTime += DeltaTime;
}

void FSkillActionStepper::StepActionTo(float FromTime, float ToTime)
{
    FActionSkillScope* PlayingSkill = Player->GetPlayingSKill();
    FromTime = FMath::Clamp(FromTime, 0.f, Action->GetActionLength());
    ToTime = FMath::Clamp(ToTime, 0.f, Action->GetActionLength());
    for (const FActionKeyFrame& Key : Action->Keys)
    {
        if (Key.Time > FromTime && Key.Time <= ToTime)
        {
            Player->OnActionKeyReached(Action, Key);
        }
    }
}

void FSkillActionStepper::ActionJumpToPosition(float ToPosition, bool bIgnoreKeyFrame)
{
    const bool bPlayForward = ToPosition > LastStepTime;
    const bool bFireNotify = !bIgnoreKeyFrame && bPlayForward;
    Player->GetSkillComponent()->SetAnimationPosition(GetActionAnimAsset(), ToPosition, bFireNotify);
    if (bFireNotify)
    {
        StepActionTo(LastStepTime, ToPosition);
    }
    LastStepTime = ToPosition;
}

UActionSkillInstance::UActionSkillInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer), CurrSkill(nullptr), bPlaying(false), Position(0.f)
{
    ActionStepper = MakeShared<FSkillActionStepper>(this);
}

UActionSkillInstance::~UActionSkillInstance()
{
    UE_LOG(LogActionSkill, Warning, TEXT("!!!! UActionSkillInstance Destroy"));
}

void UActionSkillInstance::Play(FActionSkillScope& InSkill, float InPlayRate)
{
    UE_LOG(LogActionSkill, Warning, TEXT("!!!! UActionSkillInstance::Play"));
    CurrSkill = &InSkill;
    SetPlayRate(InPlayRate);
    SkillLength = CurrSkill->CalculateSkillLength();
    if (SkillLength <= 0) { return ; }

    // 初始化技能播放环境
    Position = 0.f;
    OnSkillPlayEnd.Unbind();
    SetPlaying(true);
}

void UActionSkillInstance::PlaySingleAction(FActionSkillScope & InSkill, FAction& Action, float StartPos, float InPlayRate)
{
	CurrSkill = &InSkill;
	CurrentSelectAction = &Action;
	SetPlayRate(InPlayRate);

	if (SkillLength <= 0) { return; }

	// 初始化技能播放环境
	Position = StartPos;
	StartPosition = StartPos;
	EndPosition = StartPos + CurrentSelectAction->GetActionLength();
	OnSkillSingleActionPlayEnd.Unbind();
	this->bPlayingSingleAction = true;
}

void UActionSkillInstance::Stop(bool bInterrupt)
{
    SetPlaying(false);
    OnSkillPlayEnd.ExecuteIfBound(bInterrupt);
}

void UActionSkillInstance::StopSingleAction(bool bInterrupt)
{
	this->bPlayingSingleAction = false;
	OnSkillSingleActionPlayEnd.ExecuteIfBound(bInterrupt);
}

void UActionSkillInstance::Pause() 
{
    SetPlaying(false);
    GetSkillComponent()->PauseAnimation();
}

void UActionSkillInstance::PauseSingleAction()
{
	this->bPlayingSingleAction = false;
	GetSkillComponent()->PauseAnimation();
}

void UActionSkillInstance::SetPlaying(bool bInPlaying)
{
    this->bPlaying = bInPlaying;
}

void UActionSkillInstance::SetPosition(float Time)
{
    // Advance(Time - Position);
    // Jump to position
    if (!CurrSkill) return ;
	SkillLength = CurrSkill->CalculateSkillLength();
    float FromPosition = Position, ToPosition = FMath::Clamp(Time, 0.f, GetSkillLength());
    bool bIsPlayForward = ToPosition > FromPosition;
    float CursorTimer = 0.f;
    if (bIsPlayForward)
    {
        for (auto It = CurrSkill->CreateActionIterator(); It; ++ It)
        {
            float ActionStartTime = CursorTimer;
            float ActionEndTime = ActionStartTime + It->GetActionLength();
            if (FromPosition < ActionEndTime)
            {
                // 如果开始时间小于当前Action的起始时间，开始播放的Action
                if (FromPosition <= ActionStartTime)
                {
                    ActionStepper->InitAction(*It);
                    ActionStepper->Start();
                }

                if (ToPosition < ActionEndTime)
                {
                    ActionStepper->ActionJumpToPosition(ToPosition - ActionStartTime);
                    break;
                }

                ActionStepper->ActionJumpToPosition(It->GetActionLength());
                FromPosition = ActionEndTime;
            }
            CursorTimer = ActionEndTime;
        }
    }
    else
    {
        for (auto It = CurrSkill->CreateActionIterator(); It; ++ It)
        {
            float ActionStartTime = CursorTimer;
            float ActionEndTime = ActionStartTime + It->GetActionLength();
            if (ToPosition <= ActionEndTime)
            {
                if (FromPosition <= ActionStartTime)
                {
                    ActionStepper->InitAction(*It);
                    ActionStepper->Start();
                }
                ActionStepper->ActionJumpToPosition(ToPosition - ActionStartTime);
                break;
            }
            CursorTimer = ActionEndTime;
        }
    }

    Position = ToPosition;
}

void UActionSkillInstance::SetPlayRate(float Rate)
{
    PlayRate = Rate;
}

void UActionSkillInstance::Sample(float Time)
{
    SetPosition(Time);
}

void UActionSkillInstance::Update(float DeltaTime)
{
    if (IsPlaying())
    {
        // step skill
        Advance(DeltaTime * (IsPlayForward() ? 1 : -1) * PlayRate);
    }

	if (bPlayingSingleAction) 
	{
		AdvaceSingleAction(DeltaTime * (IsPlayForward() ? 1 : -1) * PlayRate);
	}
}

/**
 * 本方法不要做IsPlaying()的保护，调到这里一定会触发计算
 */
void UActionSkillInstance::Advance(float DeltaTime)
{
    if (!CurrSkill) return ;
    float FromPosition = Position, ToPosition = FMath::Clamp(Position + DeltaTime, 0.f, GetSkillLength());
    float CursorTimer = 0.f;
    if (DeltaTime > 0) // 正向播放
    {
        for (auto It = CurrSkill->CreateActionIterator(); It; ++ It)
        {
            float EndTime = CursorTimer + It->GetActionLength();
            if (FromPosition < EndTime)
            {
                // 如果开始时间小于当前Action的起始时间，开始播放的Action
                if (FromPosition <= CursorTimer)
                {
                    ActionStepper->InitAction(*It);
                    ActionStepper->Start();
                }

                if (ToPosition < EndTime)
                {
                    ActionStepper->Advance(ToPosition - FromPosition);
                    break;
                }

                ActionStepper->Advance(EndTime - FromPosition);
                FromPosition = EndTime;
            }
            CursorTimer = EndTime;
        }

        Position = ToPosition;
    }
    else // 逆向播放
    {
        Position = ToPosition;
    }

    if (Position >= GetSkillLength())
    {
        Stop(false);
    }
}

void UActionSkillInstance::AdvaceSingleAction(float DeltaTime)
{
	if (!CurrSkill || !CurrentSelectAction) return;
	float ToPosition = FMath::Clamp(Position + DeltaTime, StartPosition, EndPosition);

	if (DeltaTime > 0)
	{
		if (Position < EndPosition)
		{
			if (Position <= StartPosition)
			{
				ActionStepper->InitAction(*CurrentSelectAction);
				ActionStepper->Start();
			}
			ActionStepper->Advance(EndPosition - StartPosition);
		}
	}
	
	Position = ToPosition;

	if (Position >= EndPosition)
	{
		StopSingleAction(false);
	}
}

float UActionSkillInstance::GetPlayLength() const
{
    return PlayRate > 0 ? (SkillLength / PlayRate) : PlayRate;
}

class UActionSkillComponent* UActionSkillInstance::GetSkillComponent()
{ 
    return CastChecked<UActionSkillComponent>(GetOuter()); 
}

void UActionSkillInstance::OnActionKeyReached(FAction* Action, const FActionKeyFrame& KeyFrame)
{
    FActionEventPtr KeyEvent = KeyFrame.GetEvent();
    // Trigger Key
    if (KeyEvent.IsValid() && KeyEvent->Params.IsValid())
    {
        if (SkillModule == nullptr)
        {
            SkillModule = &FModuleManager::Get().GetModuleChecked<FActionSkillModule>("ActionSkill");
        }

        if (auto NotifyHandler = SkillModule->SearchActionNotifyHandler(KeyEvent->Params.GetStruct()))
        {
            NotifyHandler->ExecuteIfBound(GetSkillComponent(), GetPlayingSKill(), *Action, KeyFrame);
        }
    }
}

/*********************************************************************/
// UActionSkillComponent
/*********************************************************************/
UActionSkillComponent::UActionSkillComponent(const FObjectInitializer& ObjectInitializer)
    : UActorComponent(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;


    SkillInstance = CreateDefaultSubobject<UActionSkillInstance>(TEXT("ActionSkillInstance"));
}

void UActionSkillComponent::PlaySkill(int32 Id)
{
    FActionSkillPtr ActionSkillPtr = FActionSkillModule::Get().GetSkillById(Id);
    if (ActionSkillPtr.IsValid())
    {
        PlaySkill(*ActionSkillPtr.Get());
    }
    else
    {
        UE_LOG(LogActionSkill, Warning, TEXT("Skill with id %d can not be found"), Id);
    }
    
}

void UActionSkillComponent::PlaySkill(FActionSkillScope& Skill)
{
    SkillInstance->Play(Skill);
}

void UActionSkillComponent::PlaySkillSingleAction(FActionSkillScope & Skill, FAction & Action, float StartPos)
{
	SkillInstance->PlaySingleAction(Skill, Action, StartPos);
}

void UActionSkillComponent::PlayAnimation(FString Name, float PlayRate)
{
    // TODO: Need implementation
}

void UActionSkillComponent::PlayAnimation(class UAnimSequenceBase* AnimAsset, float PlayRate)
{
    // TODO: Need implementation
}

void UActionSkillComponent::PauseAnimation()
{
    // TODO: Need implementation
}

void UActionSkillComponent::SetAnimationPosition(UAnimSequenceBase* AnimAsset, float ToPosition, bool bFireNotify)
{
    // TODO: Need implementation
}

void UActionSkillComponent::UActionSkillComponent::PlaySound()
{
    // TODO: Need implementation
}

void UActionSkillComponent::PlayEffect()
{
    // TODO: Need implementation
}

void UActionSkillComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    if (SkillInstance) SkillInstance->Update(DeltaTime);
}

class ACharacter* UActionSkillComponent::GetCharacter()
{
    return Cast<ACharacter>(GetOwner()); 
}