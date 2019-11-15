// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-19
#pragma once

#include "CoreMinimal.h"
#include "AdvancedPreviewScene.h"
#include "EditorUndoClient.h"
#include "AdvancedPreviewSceneModule.h"

using FDetailCustomizationInfo = FAdvancedPreviewSceneModule::FDetailCustomizationInfo;
using FPropertyTypeCustomizationInfo = FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo;

class ACTIONSKILLEDITOR_API FActionSkillEditorPreviewScene : public FAdvancedPreviewScene, public FEditorUndoClient
{
public:
	FActionSkillEditorPreviewScene(ConstructionValues CVS);
	virtual ~FActionSkillEditorPreviewScene();
	void Initialize();

	//~ Begin: FEditorUndoClient interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	//~ End: FEditorUndoClient interface

	/** FPreviewScene interface */
	//~ Begin: FPreviewScene  interface
	virtual void Tick(float InDeltaTime) override;
	virtual bool IsTickable() const override;
	virtual ETickableTickType GetTickableTickType() const { return ETickableTickType::Always; }
	//~ End: FPreviewScene interface

	class ISkillPreviewProxy* GetPreviewProxy();
	void OnPreviewSceneSettingCustomization(TArray<FDetailCustomizationInfo>& DetailsCustomizations, TArray<FPropertyTypeCustomizationInfo>& PropertyTypeCustomizationInfos);
	UObject* GetPreviewSceneAddSetting();

	FORCEINLINE ACharacter* GetPrevCharacter() { return PrevCharacter; }
	FORCEINLINE ACharacter* GetPrevEnemy() { return PrevEnemy; }
	FORCEINLINE FSimpleMulticastDelegate& GetOnPreTickDelegate() { return OnPreTickDelegate; }
	FORCEINLINE FSimpleMulticastDelegate& GetOnPostTickDelegate() { return OnPostTickDelegate; }

	void SetPrevCharacter(class ACharacter* InCaster);
	void SetPrevEnemy(class ACharacter* InTarget);

private:
	ACharacter* PrevCharacter;
	ACharacter* PrevEnemy;

	FSimpleMulticastDelegate OnPreTickDelegate;
	FSimpleMulticastDelegate OnPostTickDelegate;
};