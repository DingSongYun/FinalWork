// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-16

#pragma once

#include "CoreMinimal.h"
#include "ActionSkill.h"
#include "Components/ActorComponent.h"
#include "ActionSkillComponent.generated.h"

struct FSkillActionStepper
{
public:
    FSkillActionStepper(class UActionSkillInstance* SkillPlayer);
    void InitAction(FAction& InAction);
    void Start();
    void Advance(float DeltaTime);
    void StepActionTo(float FromTime, float ToTime);
    void ActionJumpToPosition(float ToTime, bool bIgnoreKeyFrame = false);
private:
    UAnimSequenceBase* GetActionAnimAsset();
private:
    UActionSkillInstance*    Player;
    FAction*                 Action;
    float                    LastStepTime;
};


// DECLARE_DELEGATE_OneParam(FOnSkillPlayEnd, bool/*bInterrupt*/);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSkillPlayEnd, bool, bInterrupt);

UCLASS(BlueprintType, Blueprintable, ClassGroup=Skill)
class ACTIONSKILL_API UActionSkillInstance : public UObject
{
    GENERATED_BODY()
public:
    UActionSkillInstance(const FObjectInitializer& ObjectInitializer);
    ~UActionSkillInstance();

    /** 播放技能 */
    void Play(FActionSkillScope& InSkill, float InPlayRate = 1.f);
	void PlaySingleAction(FActionSkillScope& InSkill, FAction& Action, float StartPos, float InPlayRate = 1.f);
    void Stop(bool bInterrupt = true);
	void StopSingleAction(bool bInterrupt = true);
    void Pause();
	void PauseSingleAction();

    /** 当前是否在播放技能 */
    FORCEINLINE bool IsPlaying() const { return bPlaying; }
	FORCEINLINE bool IsPlayingSingleAction(FAction& Action) const { return bPlayingSingleAction && Action == *CurrentSelectAction; }
    FORCEINLINE FActionSkillScope* GetPlayingSKill() { return CurrSkill; }

    /** 设置播放状态，暂停或者继续播放 */
    void SetPlaying(bool bPlaying);

    /** 跳转播放位置 */
    void SetPosition(float Time);

    /** 设定播放速度 */
    void SetPlayRate(float Rate);
    FORCEINLINE float GetPlayRate() const { return PlayRate; }

    /** 当前技能播放到的位置 */
    float GetPosition() const { return Position; }

    /** 采样，取某个时间点的技能状态 */
    void Sample(float Time);

    /** update */
    void Update(float DeltaTime);

    /** 本方法的DeltaTime带有播放方向的，> 0 正向播放， < 0 方向播放 */
    void Advance(float DeltaTime);

	void AdvaceSingleAction(float DeltaTime);

    /** 获取技能时长 */
    float GetSkillLength() const { return SkillLength; }

    /** 获取技能实际播放长度，根据PlayRate进行缩放 */
    float GetPlayLength() const;

    /** 正向播放还是逆向播放，暂时不实现逆向播放 */
    bool IsPlayForward() const { return true; }

    /** 获取技能Component */
    class UActionSkillComponent* GetSkillComponent();

    /** 设置播放回调 */
    void SetEndDelegate(const FOnSkillPlayEnd& InOnEnded) { OnSkillPlayEnd = InOnEnded; }

    void OnActionKeyReached(FAction* Action, const FActionKeyFrame& KeyFrame);

private:
    /** 当前正在播放的技能 */
    FActionSkillScope*                      CurrSkill;
    /** 是否正在播放技能 */
    bool                                    bPlaying;
	bool                                    bPlayingSingleAction;
    /** 播放回调 */
    FOnSkillPlayEnd                         OnSkillPlayEnd;

	/** 单条Action播放回调 */
	FOnSkillPlayEnd                         OnSkillSingleActionPlayEnd;

    /** 技能时长 */
    float                                   SkillLength;
    /** 播放速率 */
    float                                   PlayRate;
    /** 当前播放到的位置 */
    float                                   Position;
	float                                   StartPosition;
	float                                   EndPosition;
    /** Action播放 */
    TSharedPtr<FSkillActionStepper>         ActionStepper;

	FAction* CurrentSelectAction;

    FActionSkillModule*      SkillModule;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=ActionSkill)
class ACTIONSKILL_API UActionSkillComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    friend struct FSkillActionStepper;
    friend class UActionSkillInstance;

    UActionSkillComponent(const FObjectInitializer& ObjectInitializer);
    class ACharacter* GetCharacter();

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    FORCEINLINE bool IsPlaying() const { return SkillInstance->IsPlaying(); }
	FORCEINLINE bool IsPlayingSingleAction(FAction& Action) const { return SkillInstance->IsPlayingSingleAction(Action); }
    FORCEINLINE void Stop(bool bInterrupt = true) { SkillInstance->Stop(bInterrupt); }
    FORCEINLINE void Pause() { SkillInstance->Pause(); }
	FORCEINLINE void PauseSingleAction() { SkillInstance->PauseSingleAction(); }
    FORCEINLINE void SetPlaying(bool bPlaying) { SkillInstance->SetPlaying(bPlaying); }
    FORCEINLINE void SetPosition(float NewPosition) { SkillInstance->SetPosition(NewPosition); }
    FORCEINLINE float GetPosition() const { return SkillInstance->GetPosition(); }
    FORCEINLINE FActionSkillScope* GetPlayingSkill() const { return SkillInstance->GetPlayingSKill(); }
    FORCEINLINE void SetEndDelegate(const FOnSkillPlayEnd& InOnEnded) { SkillInstance->SetEndDelegate(InOnEnded); }

    /// Begin: 技能播放相关接口,
    ///  我们在这只实现一个最简单的脱离项目的版本
    ///  各个项目需要根据具体业务定制自己的实现
    virtual void PlaySkill(int32 Id);
    virtual void PlaySkill(FActionSkillScope& Skill);
	virtual void PlaySkillSingleAction(FActionSkillScope& Skill, FAction& Action, float StartPos);
    // 设置技能播放速度
    virtual void SetPlayRate(float InRate) { return SkillInstance->SetPlayRate(InRate); }
    FORCEINLINE float GetPlayRate() const { return SkillInstance->GetPlayRate(); }

protected:
    /** 播放技能动画 */
    virtual void PlayAnimation(FString Name, float PlayRate = 1);
    virtual void PlayAnimation(class UAnimSequenceBase* AnimAsset, float PlayRate = 1);
    virtual void PauseAnimation();
    /* 暂时只在preview时用到本方法 */
    virtual void SetAnimationPosition(class UAnimSequenceBase* AnimAsset, float ToPosition, bool bFireNotify);

    /** 播放声音 */
    virtual void PlaySound();

    /** 播放特效 */
    virtual void PlayEffect();

    /// End: 技能播放相关接口,

protected:
    UPROPERTY(EditAnywhere)
    UActionSkillInstance*       SkillInstance;
};