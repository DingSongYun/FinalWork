#include "SkillEditorProxy.h"
#include "LuaBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "SkillEditorMiddleware.h"
#include "ActionSkillEditor.h"
#include "ActionSkillEditorPreviewScene.h"
#include "CppBinding/CppBindingLibs.h"
#include "LuaCppBinding.h"
#include "LuaWrapper/CustomLuaWrapper.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "SkillPreviewSceneCustomization.h"
#include "EditorUtilities.h"
#include "Engine/DirectionalLight.h"
#include "Engine/LevelStreaming.h"
#include "EditorLevelUtils.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/Classes/PhysicsEngine/PhysicsAsset.h"
#include "ActionSkillComponent.h"
#include "Engine/LevelStreaming.h"
#include "LevelUtils.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace LuaSkillEdInterface
{
    const FString PreInitializePrevScene                = "K2_PreInitializePrevScene";
    const FString InitializeEditor                      = "K2_InitializeEditor";
    const FString CreateSkillPreviewCharacter           = "K2_CreateCharacter";
    const FString AssembleCharacter                     = "K2_AssembleCharacter";
    const FString RemoveCharacter                       = "K2_RemoveCharacter";
    const FString PlayCharacterSkill                    = "K2_PlayCharacterSkill";
};

struct FLuaProxyImpl
{
    static void OnPreviewContextInitialized(USkillEditorProxy* Proxy, class UWorld* InWorld)
    {
        TArray<FLuaBPVar> Args;
        Args.Add(ULuaBlueprintLibrary::CreateVarFromObject(Proxy));
        Args.Add(ULuaBlueprintLibrary::CreateVarFromObject(InWorld));
        Proxy->CallLuaFunction_Call(LuaSkillEdInterface::PreInitializePrevScene, Args, true);
    }

    static ACharacter* CreateCharacter(USkillEditorProxy* Proxy, int32 cfgId)
    {
        TArray<FLuaBPVar> Args;
        Args.Add(ULuaBlueprintLibrary::CreateVarFromInt(cfgId));
        FLuaBPVar Result = Proxy->CallLuaFunction_Call(LuaSkillEdInterface::CreateSkillPreviewCharacter, Args, true);

        return Cast<ACharacter>(ULuaBlueprintLibrary::GetObjectFromVar(Result));
    }

    static void RemoveCharacter(USkillEditorProxy* Proxy, class ACharacter* InCharacter)
    {
        if (InCharacter == nullptr) return;

        TArray<FLuaBPVar> Args {
            ULuaBlueprintLibrary::CreateVarFromObject(InCharacter),
        };
        Proxy->CallLuaFunction_Call(LuaSkillEdInterface::RemoveCharacter, Args, true);
    }

    static void AssembleCharacter(USkillEditorProxy* Proxy, class ACharacter* InCharacter, int32 NewId)
    {
        if (InCharacter == nullptr) return;

        TArray<FLuaBPVar> Args {
            ULuaBlueprintLibrary::CreateVarFromObject(InCharacter),
            ULuaBlueprintLibrary::CreateVarFromInt(NewId)
        };
        Proxy->CallLuaFunction_Call(LuaSkillEdInterface::AssembleCharacter, Args, true);
    }

    static void PlayCharacterSkill(USkillEditorProxy* Proxy, class ACharacter* InCharacter, int32 SkillId)
    {
        if (InCharacter == nullptr) return;

        TArray<FLuaBPVar> Args {
            ULuaBlueprintLibrary::CreateVarFromObject(InCharacter),
            ULuaBlueprintLibrary::CreateVarFromInt(SkillId)
        };
        Proxy->CallLuaFunction_Call(LuaSkillEdInterface::PlayCharacterSkill, Args, true);
    }
};

USkillEditorProxy::USkillEditorProxy(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , PrevCharacterId(100101)
    , PrevEnemyId(100101)
{
}

void USkillEditorProxy::OnPreviewContextInitialized(class UWorld* World)
{
    // SetLuaEditorWorld(World);
    EdMapLevel = EditorLevelUtils::AddLevelToWorld(World, *FString("/Game/Maps/Editor/SkillEditor"), ULevelStreamingDynamic::StaticClass());
    if (EdMapLevel)
    // if (EdMapLevel = EditorLevelUtils::CreateNewStreamingLevelForWorld(*World, ULevelStreamingDynamic::StaticClass()))
    {
        // Do something for level
        EdMapLevel->SetFlags(RF_Transactional);
        World->SetCurrentLevel(World->PersistentLevel);
    }
    FLuaProxyImpl::OnPreviewContextInitialized(this, World);
}

void USkillEditorProxy::OnTerminated()
{
    if (EdMapLevel)
    {
        EditorLevelUtils::RemoveLevelFromWorld(EdMapLevel->GetLoadedLevel());
    }
}

void USkillEditorProxy::Initialize()
{
    if (FActionSkillEditorPreviewScene* PreviewScene = GetPreviewScene())
    {
        PreviewScene->GetWorld()->ChangeFeatureLevel(ERHIFeatureLevel::ES3_1, false);
    }
    CallLuaFunctionSimpleImpl(LuaSkillEdInterface::InitializeEditor, true);
}

ACharacter* USkillEditorProxy::CreatePrevCharacter(class UWorld* WorldContext)
{
    return CreateCharacter(WorldContext, PrevCharacterId);
}

ACharacter* USkillEditorProxy::CreatePrevEnemy(class UWorld* WorldContext)
{
    // ACharacter* Result = CreateCharacter(this, PrevEnemyId);
    // return Result;
    return nullptr;
}

ACharacter* USkillEditorProxy::CreateCharacter(UWorld* WorldContext, int32 CharaId)
{
    ACharacter* NewCharacter = FLuaProxyImpl::CreateCharacter(this, PrevCharacterId);
    if (WorldContext)
    {
        if ( UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(NewCharacter->GetComponentByClass(UCharacterMovementComponent::StaticClass())) )
        {
            MoveComp->SetUpdatedComponent(NewCharacter->GetRootComponent());
            // MoveComp->CharacterOwner = NewCharacter;
        }
    }

    return NewCharacter;
}

void USkillEditorProxy::OnCustomizePrevSceneDetailSettings(TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo>& DetailsCustomizations)
{
    DetailsCustomizations.Add({ USkillEditorProxy::StaticClass(), FOnGetDetailCustomizationInstance::CreateLambda([&]() -> TSharedRef<IDetailCustomization>
        {
            return MakeShareable(new FSkillPreviewSceneDetailCustomization(this));
        }
    )});
}

void USkillEditorProxy::OnCustomizePrevScenePropertySettings(TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo>& PropertyTypeCustomizationInfos)
{}

UObject* USkillEditorProxy::GetPreviewSceneAddSetting()
{
    return this;
}

void USkillEditorProxy::PlayActionSkill(int32 SkillId)
{
    FLuaProxyImpl::PlayCharacterSkill(this, GetPrevCharacter(), SkillId);
}

FActionSkillEditorPreviewScene* USkillEditorProxy::GetPreviewScene()
{
    if (TSharedPtr<class FActionSkillEditor> SkillEditor = FSkillEditorMiddleware::GetSkillEditor())
    {
        return SkillEditor->GetPrviewScene().Get();
    }
    return nullptr;
}

void USkillEditorProxy::ChangePrevCharacter(int32 NewId)
{
    if (PrevCharacterId != NewId)
    {
        PrevCharacterId = NewId;
        if (auto OldCharacter = GetPreviewScene()->GetPrevCharacter())
        {
            FLuaProxyImpl::RemoveCharacter(this, OldCharacter);
            GetPreviewScene()->GetWorld()->DestroyActor(OldCharacter);
        }
        auto NewCharacter = CreatePrevCharacter(GetPreviewScene()->GetWorld());
        GetPreviewScene()->SetPrevCharacter(NewCharacter);
        HandleOnCharacterChanged(NewCharacter);
    }
}

void USkillEditorProxy::ChangePrevEnemy(int32 NewId)
{
    if (PrevEnemyId != NewId)
    {
        PrevEnemyId = NewId;
        if (auto OldCharacter = GetPreviewScene()->GetPrevEnemy())
        {
            FLuaProxyImpl::RemoveCharacter(this, OldCharacter);
            GetPreviewScene()->GetWorld()->DestroyActor(OldCharacter);
        }
        auto NewCharacter = CreatePrevEnemy(GetPreviewScene()->GetWorld());
        GetPreviewScene()->SetPrevEnemy(NewCharacter);
        HandleOnCharacterChanged(NewCharacter);
    }
}

void USkillEditorProxy::HandleOnCharacterChanged(class ACharacter* NewCharacter)
{
    if (!NewCharacter) return ;
    for (UActorComponent* Component : NewCharacter->GetComponentsByClass(USkeletalMeshComponent::StaticClass()))
    // if (USkeletalMeshComponent* MeshComponent = NewCharacter->GetMesh())
    {
        USkeletalMeshComponent* MeshComponent = Cast<USkeletalMeshComponent>(Component);
        if (UPhysicsAsset* PhysAsset = MeshComponent->GetPhysicsAsset())
        {
            PhysAsset->InvalidateAllPhysicsMeshes();
            MeshComponent->TermArticulated();
            MeshComponent->InitArticulated(GetPreviewScene()->GetWorld()->GetPhysicsScene());

            static FName CollisionProfileName(TEXT("PhysicsActor"));
            MeshComponent->SetCollisionProfileName(CollisionProfileName);
        }
    }
}

class UDirectionalLightComponent* USkillEditorProxy::GetDirectionLightComponent()
{
    return GetPreviewScene()->DirectionalLight;
}

DEF_CLASS_EXTENSION(SkillEditorProxy)
    REG_EXTENSION_METHOD_IMP(USkillEditorProxy, "GetDirectionLight", {
        CheckUD(USkillEditorProxy, L, 1);
        UDirectionalLightComponent* Result = UD->GetPreviewScene()->DirectionalLight;
        return  LuaObject::push(L, Result);
    });
END_CLASS_EXTENSION(SkillEditorProxy)
