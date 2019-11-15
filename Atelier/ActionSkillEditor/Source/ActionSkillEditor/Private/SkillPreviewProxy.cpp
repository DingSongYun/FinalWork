#include "ISkillPreviewProxy.h"
#include "ActionSkill.h"
#include "GameFramework/Character.h"
#include "ActionSkillComponent.h"
#include "ActionSkillEditor.h"
#include "ActionSkillEditorModule.h"

void SkillPreviewProxyBase::PlayActionSkill(int32 SkillId)
{
    auto Skill = FActionSkillModule::Get().GetSkillById(SkillId);
    if (!Skill.IsValid()) return;

    if ( UActionSkillComponent* SkillComp = GetPrevSkillComponent() )
    {
        SkillComp->PlaySkill(*Skill.Get());
    }
}

void SkillPreviewProxyBase::PlayActionSkillSingleAction(int32 SkillId, FAction& Action, float StartPos)
{
	auto Skill = FActionSkillModule::Get().GetSkillById(SkillId);
	if (!Skill.IsValid()) return;

	if (UActionSkillComponent* SkillComp = GetPrevSkillComponent())
	{
		SkillComp->PlaySkillSingleAction(*Skill.Get(), Action, StartPos);
	}
}

void SkillPreviewProxyBase::SetPlayRate(float InRate)
{
    if ( UActionSkillComponent* SkillComp = GetPrevSkillComponent() )
    {
        SkillComp->SetPlayRate(InRate);
    }
}

bool SkillPreviewProxyBase::IsPlayingActionSkill()
{
    if ( UActionSkillComponent* SkillComp = GetPrevSkillComponent() )
    {
        return SkillComp->IsPlaying();
    }

    return false;
}

bool SkillPreviewProxyBase::IsPlayingActionSkillSingleAction(FAction& Action)
{
	if (UActionSkillComponent* SkillComp = GetPrevSkillComponent())
	{
		return SkillComp->IsPlayingSingleAction(Action);
	}

	return false;
}

void SkillPreviewProxyBase::PauseActionSkill()
{
    if (IsPlayingActionSkill())
    {
        if ( UActionSkillComponent* SkillComp = GetPrevSkillComponent() )
        {
            SkillComp->Pause();
        }
    }
}

void SkillPreviewProxyBase::PauseActionSkillSingleAction()
{
	if (UActionSkillComponent* SkillComp = GetPrevSkillComponent())
	{
		SkillComp->PauseSingleAction();
	}
}

void SkillPreviewProxyBase::SampleActionSkillAtPosition(int32 SkillId, float InPosition)
{
    auto Skill = FActionSkillModule::Get().GetSkillById(SkillId);
    if (Skill.IsValid())
    {
        if (auto SkillComp = GetPrevSkillComponent())
        {
            if (SkillComp->GetPlayingSkill() != Skill.Get())
            {
                PlayActionSkill(SkillId);
            }
            PauseActionSkill();
            SkillComp->SetPosition(InPosition);
        }
    }
}

TWeakPtr<class FActionSkillEditor> SkillPreviewProxyBase::GetSkillEditor()
{
    IActionSkillEditorModule& ActionSkillModule = FModuleManager::GetModuleChecked<IActionSkillEditorModule>(TEXT("ActionSkillEditor"));
    return ActionSkillModule.GetSingletonSkillEditor();
}

ACharacter* SkillPreviewProxyBase::GetPrevCharacter()
{
    TWeakPtr<class FActionSkillEditor> ActionSkillEditor = GetSkillEditor();
    if (ActionSkillEditor.IsValid())
    {
        return ActionSkillEditor.Pin()->GetPreviewSkillCaster();
    }
    return nullptr;
}

UActionSkillComponent* SkillPreviewProxyBase::GetPrevSkillComponent()
{
    if (ACharacter* Caster = GetPrevCharacter()) {
        return Cast<UActionSkillComponent>(Caster->GetComponentByClass(UActionSkillComponent::StaticClass()));
    }

    return nullptr;
}