// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-15

#pragma once

#include "CoreMinimal.h"
#include "Lua/ToLuaInterface.h"
#include "ISkillPreviewProxy.h"
#include "ActionSkillEditor.h"
#include "SkillEditorProxy.generated.h"

UCLASS()
class ATELIEREDITOR_API USkillEditorProxy : public UObject, public IToLuaInterface, public SkillPreviewProxyBase
{
    friend class FSkillPreviewSceneDetailCustomization;

	GENERATED_UCLASS_BODY()
	GENERATED_LUAINTERFACE_BODY()
public:
    void Initialize();

    //~ Begin: ISkillPreviewProxy Interface
    class ACharacter* CreatePrevCharacter(class UWorld* WorldContext) override;
    class ACharacter* CreatePrevEnemy(class UWorld* WorldContext) override;
    void OnPreviewContextInitialized(class UWorld* World) override;
    void OnTerminated() override;
    void OnCustomizePrevSceneDetailSettings(TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo>& DetailsCustomizations) override;
    void OnCustomizePrevScenePropertySettings(TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo>& PropertyTypeCustomizationInfos) override;
    UObject* GetPreviewSceneAddSetting() override;
    void PlayActionSkill(int32 SkillId) override;
    //~ End: ISkillPreviewProxy Interface

    class FActionSkillEditorPreviewScene* GetPreviewScene();

    UFUNCTION(BlueprintCallable)
    class UDirectionalLightComponent* GetDirectionLightComponent();

private:
    class ACharacter* CreateCharacter(class UWorld* WorldContext, int32 CharaId);
    void ChangePrevCharacter(int32 NewId);
    void ChangePrevEnemy(int32 NewId);
    void HandleOnCharacterChanged(class ACharacter* InCharacter);

private:
    int32 PrevCharacterId;
    int32 PrevEnemyId;
    class ULevelStreaming* EdMapLevel;
};
