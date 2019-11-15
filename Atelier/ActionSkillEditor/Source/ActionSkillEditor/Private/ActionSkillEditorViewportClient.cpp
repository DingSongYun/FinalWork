// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-24

#include "ActionSkillEditorViewportClient.h"
#include "Modules/ModuleManager.h"
#include "ActionSkillEditorModule.h"
#include "ActionSkillEditorPreviewScene.h"
#include "TabFactory/SActionSkillViewport.h"
#include "EditorModeManager.h"
#include "AssetEditorModeManager.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

#define LOCTEXT_NAMESPACE "FActionSkillEdViewportClient"

FActionSkillEdViewportClient::FActionSkillEdViewportClient(const TSharedRef<class FActionSkillEditorPreviewScene>& InPreviewScene, const TSharedRef<class SActionSkillEdViewport>& InViewport, const TSharedRef<class FAssetEditorToolkit>& InAssetEditorToolkit, int32 InViewportIndex)
	: FEditorViewportClient(nullptr, &InPreviewScene.Get(), StaticCastSharedRef<SEditorViewport>(InViewport))
	, PreviewScenePtr(InPreviewScene)
	, AssetEditorToolkitPtr(InAssetEditorToolkit)
	, ViewportIndex(InViewportIndex)
	, CameraFollowMode(ESkillViewportCameraFollowMode::None)
	, CameraFollowBoneName(NAME_None)
	, OrbitRotation(FQuat::Identity)
{
	bOwnsModeTools = true;
	InAssetEditorToolkit->SetAssetEditorModeManager((FAssetEditorModeManager*)ModeTools);
	Widget->SetUsesEditorModeTools(ModeTools);
	((FAssetEditorModeManager*)ModeTools)->SetPreviewScene(&InPreviewScene.Get());
	ModeTools->SetWidgetMode(FWidget::WM_Rotate);
	// ((FAssetEditorModeManager*)ModeTools)->SetDefaultMode(FPersonaEditModes::SkeletonSelection);

	// Default to local space
	SetWidgetCoordSystemSpace(COORD_Local);

	// Config
	EngineShowFlags.Game = 0;
	EngineShowFlags.ScreenSpaceReflections = 1;
	EngineShowFlags.AmbientOcclusion = 1;
	EngineShowFlags.SetSnap(0);
	EngineShowFlags.Grid = false;
	EngineShowFlags.SetSeparateTranslucency(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);
	EngineShowFlags.SetSelectionOutline(true);
	EngineShowFlags.EnableAdvancedFeatures();
	EngineShowFlags.AntiAliasing = false;

	SetRealtime(true);
	if(GEditor->PlayWorld)
	{
		SetRealtime(false,true);
	}
	SetGameView(true);

	PreviewScenePtr.Pin()->GetOnPreTickDelegate().Add(FSimpleDelegate::CreateRaw(this, &FActionSkillEdViewportClient::HandlerPreviewScenePreTick));
	PreviewScenePtr.Pin()->GetOnPostTickDelegate().Add(FSimpleDelegate::CreateRaw(this, &FActionSkillEdViewportClient::HandlerPreviewScenePostTick));

	FOVAngle = 70.f;
	ViewFOV = 70.f;
}

FActionSkillEdViewportClient::~FActionSkillEdViewportClient()
{
	if (PreviewScenePtr.IsValid())
	{
		PreviewScenePtr.Pin()->GetOnPreTickDelegate().RemoveAll(this);
		PreviewScenePtr.Pin()->GetOnPostTickDelegate().RemoveAll(this);
	}
}

void FActionSkillEdViewportClient::SetCameraFollowMode(ESkillViewportCameraFollowMode InCameraFollowMode, FName InBoneName)
{
	bool bCanFollow = true;
	auto SkillCasterCharacter = PreviewScenePtr.Pin()->GetPrevCharacter();
	USkeletalMeshComponent* PreviewMeshComponent = SkillCasterCharacter ? SkillCasterCharacter->GetMesh() : nullptr;
	if (!PreviewMeshComponent) return ;

	if(InCameraFollowMode == ESkillViewportCameraFollowMode::Bone && PreviewMeshComponent)
	{
		bCanFollow = PreviewMeshComponent->GetBoneIndex(InBoneName) != INDEX_NONE;
	}

	if(bCanFollow && InCameraFollowMode != ESkillViewportCameraFollowMode::None)
	{
		CameraFollowMode = InCameraFollowMode;
		CameraFollowBoneName = InBoneName;

		bCameraLock = true;
		bUsingOrbitCamera = true;

		if (PreviewMeshComponent != nullptr)
		{
			switch(CameraFollowMode)
			{
			case ESkillViewportCameraFollowMode::Bounds:
				{
					FBoxSphereBounds Bound = PreviewMeshComponent->CalcBounds(PreviewMeshComponent->GetComponentTransform());
					SetLookAtLocation(Bound.Origin, true);
					OrbitRotation = FQuat::Identity;
				}
				break;
			case ESkillViewportCameraFollowMode::Bone:
				{
					FVector BoneLocation = PreviewMeshComponent->GetBoneLocation(InBoneName);
					SetLookAtLocation(BoneLocation, true);
					OrbitRotation = PreviewMeshComponent->GetBoneQuaternion(InBoneName) * FQuat(FVector(0.0f, 1.0f, 0.0f), PI * 0.5f);
				}
				break;
			}
		}

		SetViewLocation(GetViewTransform().ComputeOrbitMatrix().Inverse().GetOrigin());
	}
	else
	{
		CameraFollowMode = ESkillViewportCameraFollowMode::None;
		CameraFollowBoneName = NAME_None;

		OrbitRotation = FQuat::Identity;
		EnableCameraLock(false);
		// FocusViewportOnPreviewMesh(false);
		Invalidate();
	}
}

void FActionSkillEdViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);
}

void FActionSkillEdViewportClient::HandlerPreviewScenePreTick()
{
	RelativeViewLocation = FVector::ZeroVector;

	auto SkillCasterCharacter = PreviewScenePtr.Pin()->GetPrevCharacter();
	USkeletalMeshComponent* PreviewMeshComponent = SkillCasterCharacter ? SkillCasterCharacter->GetMesh() : nullptr;
	if (CameraFollowMode != ESkillViewportCameraFollowMode::None && PreviewMeshComponent != nullptr)
	{
		switch(CameraFollowMode)
		{
		case ESkillViewportCameraFollowMode::Bounds:
			{
				FBoxSphereBounds Bound = PreviewMeshComponent->CalcBounds(PreviewMeshComponent->GetComponentTransform());
				RelativeViewLocation = Bound.Origin - GetViewLocation();
			}
			break;
		case ESkillViewportCameraFollowMode::Bone:
			{
				int32 BoneIndex = PreviewMeshComponent->GetBoneIndex(CameraFollowBoneName);
				if(BoneIndex != INDEX_NONE)
				{
					FTransform BoneTransform = PreviewMeshComponent->GetBoneTransform(BoneIndex);
					RelativeViewLocation = BoneTransform.InverseTransformVector(BoneTransform.GetLocation() - GetViewLocation());
				}
			}
			break;
		}
	}
}

void FActionSkillEdViewportClient::HandlerPreviewScenePostTick()
{
	auto SkillCasterCharacter = PreviewScenePtr.Pin()->GetPrevCharacter();
	USkeletalMeshComponent* PreviewMeshComponent = SkillCasterCharacter ? SkillCasterCharacter->GetMesh() : nullptr;
	if (CameraFollowMode != ESkillViewportCameraFollowMode::None && PreviewMeshComponent != nullptr)
	{
		switch(CameraFollowMode)
		{
		case ESkillViewportCameraFollowMode::Bounds:
			{
				FBoxSphereBounds Bound = PreviewMeshComponent->CalcBounds(PreviewMeshComponent->GetComponentTransform());
				SetViewLocation(Bound.Origin + RelativeViewLocation);
				SetLookAtLocation(Bound.Origin);
			}
			break;
		case ESkillViewportCameraFollowMode::Bone:
			{
				int32 BoneIndex = PreviewMeshComponent->GetBoneIndex(CameraFollowBoneName);
				if(BoneIndex != INDEX_NONE)
				{
					FTransform BoneTransform = PreviewMeshComponent->GetBoneTransform(BoneIndex);
					if(IsPerspective())
					{
						// SetViewLocation(BoneTransform.GetLocation() - BoneTransform.TransformVector(RelativeViewLocation));
						SetViewLocation(BoneTransform.GetLocation());
						SetViewRotation(BoneTransform.GetRotation().Rotator());
					}
					else
					{
						SetViewLocation(BoneTransform.GetLocation());
					}
					SetLookAtLocation(BoneTransform.GetLocation());
					OrbitRotation = BoneTransform.GetRotation() * FQuat(FVector(0.0f, 1.0f, 0.0f), PI * 0.5f);
				}
			}
			break;
		}
	}
}
#undef LOCTEXT_NAMESPACE