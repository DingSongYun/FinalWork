// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-19

#include "ActionSkillEditorPreviewScene.h"
#include "ModuleManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/WorldSettings.h"
#include "Components/CapsuleComponent.h"
#include "ActionSkillEditorModule.h"
#include "ISkillPreviewProxy.h"
#include "Components/SkeletalMeshComponent.h"
#include "AI/NavigationSystemBase.h"
#include "AI/NavigationSystemConfig.h"
#include "EngineUtils.h"

FActionSkillEditorPreviewScene::FActionSkillEditorPreviewScene(ConstructionValues CVS)
	: FAdvancedPreviewScene(CVS)
	, PrevCharacter(nullptr), PrevEnemy(nullptr)
{
	if (GEditor)
	{
		GEditor->RegisterForUndo(this);
	}

	AWorldSettings* WorldSetting = GetWorld()->GetWorldSettings(true);
	WorldSetting->bEnableWorldBoundsChecks = false;
	WorldSetting->SetFlags(RF_Transactional);
	// Hide Floor
	SetFloorOffset(1000);
	SetFloorVisibility(false, true);

	// Hide Environment
	SetEnvironmentVisibility(false, false);
}

FActionSkillEditorPreviewScene::~FActionSkillEditorPreviewScene()
{
	// clear actor
	UWorld* PrevWorld = GetWorld();
	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
	}

	//for ( TActorIterator<AActor> It(PrevWorld); It; ++It )
	//{
	//	AActor* Actor = *It;
	//	if ( !Actor->IsPendingKillPending() && Actor->GetLevel() == PrevWorld->PersistentLevel)
	//	{
	//		Actor->MarkPendingKill();
	//	}
	//}
}

void FActionSkillEditorPreviewScene::Initialize()
{
	UWorld* World = GetWorld();

	ISkillPreviewProxy* PrevProxy = GetPreviewProxy();
	PrevProxy->OnPreviewContextInitialized(World);
	SetPrevCharacter(PrevProxy->CreatePrevCharacter(World));
	SetPrevEnemy(PrevProxy->CreatePrevEnemy(World));

#if 0
	GEditor->PlayInEditor(World, true);
#else
	FNavigationSystem::AddNavigationSystemToWorld(*World, FNavigationSystemRunMode::EditorMode);
	World->CreateAISystem();
	World->FlushLevelStreaming(EFlushLevelStreamingType::Visibility);
	World->BeginPlay();
#endif
}

//~ Begin: FEditorUndoClient interface
void FActionSkillEditorPreviewScene::PostUndo(bool bSuccess)
{

}

void FActionSkillEditorPreviewScene::PostRedo(bool bSuccess)
{
}

void FActionSkillEditorPreviewScene::Tick(float InDeltaTime)
{
	OnPreTickDelegate.Broadcast();

	FAdvancedPreviewScene::Tick(InDeltaTime);

	if (!GIntraFrameDebuggingGameThread)
	{
		GetWorld()->Tick(LEVELTICK_All, InDeltaTime);
	}

	OnPostTickDelegate.Broadcast();
}

bool FActionSkillEditorPreviewScene::IsTickable() const 
{
	return true;
}

ISkillPreviewProxy* FActionSkillEditorPreviewScene::GetPreviewProxy()
{
	IActionSkillEditorModule& ActionSkillModule = FModuleManager::GetModuleChecked<IActionSkillEditorModule>(TEXT("ActionSkillEditor"));
	ISkillPreviewProxy* Proxy = ActionSkillModule.GetSkillPreviewProxy(GetWorld());

	return Proxy;
}

void FActionSkillEditorPreviewScene::SetPrevCharacter(class ACharacter* InCaster)
{
	if (InCaster == nullptr) return;
	PrevCharacter = InCaster;
	float HalfHeight = InCaster->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	InCaster->SetActorLocation(FVector(0, 0, HalfHeight));
	InCaster->SetActorRotation(FRotator(0));
}

void FActionSkillEditorPreviewScene::SetPrevEnemy(class ACharacter* InTarget)
{
	if (InTarget == nullptr) return;
	PrevEnemy = InTarget;
	float HalfHeight = InTarget->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	InTarget->SetActorLocation(FVector(300, 0, HalfHeight));
	InTarget->SetActorRotation(FRotator(0, -180, 0));
}
//~ End: FEditorUndoClient interface

void FActionSkillEditorPreviewScene::OnPreviewSceneSettingCustomization(
	TArray<FDetailCustomizationInfo>& DetailsCustomizations, 
	TArray<FPropertyTypeCustomizationInfo>& PropertyTypeCustomizationInfos)
{
	GetPreviewProxy()->OnCustomizePrevSceneDetailSettings(DetailsCustomizations);
	GetPreviewProxy()->OnCustomizePrevScenePropertySettings(PropertyTypeCustomizationInfos);
}

UObject* FActionSkillEditorPreviewScene::GetPreviewSceneAddSetting()
{
	return GetPreviewProxy()->GetPreviewSceneAddSetting();
}