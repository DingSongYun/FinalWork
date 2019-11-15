// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-12

#pragma once
#include "AdvancedPreviewSceneModule.h"
#include "ActionSkillTypes.h"

class ISkillPreviewProxy
{
public:
    virtual void OnInitialized() = 0;
    virtual void OnTerminated() = 0;
    virtual void OnPreviewContextInitialized(class UWorld* World) = 0;

    //~ Begin: For PreviewScene
    virtual class ACharacter* CreatePrevCharacter(class UWorld* WorldContext) = 0;
    virtual class ACharacter* CreatePrevEnemy(class UWorld* WorldContext) = 0;
    virtual void OnCustomizePrevSceneDetailSettings(TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo>& DetailsCustomizations) = 0;
    virtual void OnCustomizePrevScenePropertySettings(TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo>& PropertyTypeCustomizationInfos) = 0;
    virtual UObject* GetPreviewSceneAddSetting() = 0;
    //~ End: For PreviewScene

    //~ Begin: For Skill Preview
    virtual void PlayActionSkill(int32 SkillId) = 0;
	virtual void PlayActionSkillSingleAction(int32 SkillId, FAction& Action, float StartPos) = 0;
    virtual void SetPlayRate(float InRate) = 0;
    virtual bool IsPlayingActionSkill() = 0;
	virtual bool IsPlayingActionSkillSingleAction(FAction& Action) = 0;
    virtual void PauseActionSkill() = 0;
	virtual void PauseActionSkillSingleAction() = 0;
    virtual void SampleActionSkillAtPosition(int32 SkillId, float InPosition) = 0;
    //~ End: For Skill Preview
};

class ACTIONSKILLEDITOR_API SkillPreviewProxyBase : public ISkillPreviewProxy
{
public:
    virtual void OnInitialized() {}
    virtual void OnTerminated() {}
    virtual void OnPreviewContextInitialized(class UWorld* World) override {}

    //~ Begin: For PreviewScene
    virtual class ACharacter* CreatePrevCharacter(class UWorld* WorldContext) override { return nullptr; }
    virtual class ACharacter* CreatePrevEnemy(class UWorld* WorldContext) override { return nullptr; }
    virtual void OnCustomizePrevSceneDetailSettings(TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo>& DetailsCustomizations) override {}
    virtual void OnCustomizePrevScenePropertySettings(TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo>& PropertyTypeCustomizationInfos) override {}
    virtual UObject* GetPreviewSceneAddSetting() { return nullptr; }
    //~ End: For PreviewScene

    //~ Begin: For Skill Preview
    virtual void PlayActionSkill(int32 SkillId) override;
	virtual void PlayActionSkillSingleAction(int32 SkillId, FAction& Action, float StartPos) override;
    virtual void SetPlayRate(float InRate);
    virtual bool IsPlayingActionSkill() override;
	virtual bool IsPlayingActionSkillSingleAction(FAction& Action) override;
    virtual void PauseActionSkill() override;
	virtual void PauseActionSkillSingleAction() override;
    virtual void SampleActionSkillAtPosition(int32 SkillId, float InPosition) override;

    //~ End: For Skill Preview
protected:
    TWeakPtr<class FActionSkillEditor> GetSkillEditor();
    class ACharacter* GetPrevCharacter();
    class UActionSkillComponent* GetPrevSkillComponent();
};