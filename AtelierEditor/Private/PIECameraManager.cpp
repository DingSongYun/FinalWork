#include "PIECameraManager.h"
#include "Engine/Engine.h"
#include "LevelEditorViewport.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraTypes.h"
#include "Camera/CameraModifier.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraModifier_CameraTween.h"
#include "Editor.h"

/****************************************************/
/**************AOfflineCameraController**************/
/****************************************************/
AOfflineCameraController::AOfflineCameraController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
#endif
}

 AOfflineCameraController::~AOfflineCameraController()
{
}

void AOfflineCameraController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AOfflineCameraController::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);
	UpdateCamera(DeltaTime);
	SetupViewport(POV);
}

void AOfflineCameraController::PostInitialize()
{
	SpawnCameraActor();
	SpawnCameraManager();

	CameraTweenMod = NewObject<UCameraModifier_CameraTween>(this, UCameraModifier_CameraTween::StaticClass(), "CameraTween");
	AddCameraModifierToList(CameraTweenMod);

	BP_PostInitialize();
}

void AOfflineCameraController::SpawnCameraActor()
{
	if (InspectorCamera == nullptr)
	{
		InspectorCamera = GetWorld()->SpawnActor<ACameraActor>();
	}

	if (InspectorCamera)
	{
		InspectorCamera->SetActorLabel("InspectorCamera");
		InspectorCamera->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	}

	if (ThirdPlayerCamera == nullptr)
	{
		ThirdPlayerCamera = GetWorld()->SpawnActor<ACameraActor>();
	}

	if (ThirdPlayerCamera)
	{
		ThirdPlayerCamera->SetActorLabel("ThridPlayerCamera");
		ThirdPlayerCamera->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	}
}

void AOfflineCameraController::SpawnCameraManager()
{

	AOfflineCameraController* CameraManager = this;
	if (CameraManager && InspectorCamera)
	{
		CameraManager->POV.Location = InspectorCamera->GetActorLocation();
		CameraManager->POV.Rotation = InspectorCamera->GetActorRotation();
		if (UCameraComponent* CamComp = InspectorCamera->GetCameraComponent())
		{
			CameraManager->POV.FOV = CamComp->FieldOfView;
			CameraManager->POV.ProjectionMode = CamComp->ProjectionMode;
		}
	}
}

void AOfflineCameraController::PossessViewport()
{
	for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
	{
		if ((LevelVC == nullptr) || !LevelVC->IsPerspective() || !LevelVC->AllowsCinematicControl())
		{
			continue;
		}

		if (InspectorCamera!= nullptr)
		{
			//UpdatePreviewLevelViewportClientFromCameraCut(*LevelVC, CameraObject, bJumpCut);
			LockViewportToCamera(LevelVC, InspectorCamera);
		}
	}
}

void AOfflineCameraController::UnpossessViewport()
{
	for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
	{
		if ((LevelVC == nullptr) || !LevelVC->IsPerspective() || !LevelVC->AllowsCinematicControl())
		{
			continue;
		}

		if (LevelVC->IsAnyActorLocked())
		{
			LevelVC->SetActorLock(nullptr);
		}
	}
}

void AOfflineCameraController::LockViewportToCamera(FLevelEditorViewportClient* InViewportClient, ACameraActor* Camera) 
{
	InViewportClient->SetViewLocation(Camera->GetActorLocation());
	InViewportClient->SetViewRotation(Camera->GetActorRotation());

	UCameraComponent* CameraComp = Camera->GetCameraComponent();
	InViewportClient->RemoveCameraRoll();
	InViewportClient->AspectRatio = CameraComp->AspectRatio;
	InViewportClient->ViewFOV = CameraComp->FieldOfView;
	InViewportClient->UpdateViewForLockedActor();

	InViewportClient->SetActorLock(Camera);
}

void AOfflineCameraController::UpdateCamera(float DeltaTime)
{
	if (InspectorCamera)
	{
		POV.Location = InspectorCamera->GetActorLocation();
		POV.Rotation = InspectorCamera->GetActorRotation();
		if (UCameraComponent* CamComp = InspectorCamera->GetCameraComponent())
		{
			POV.FOV = CamComp->FieldOfView;
			POV.ProjectionMode = CamComp->ProjectionMode;
		}
	}
	FMinimalViewInfo& NewPOV = POV;

	UpdateViewInfo(NewPOV, DeltaTime);
}

void AOfflineCameraController::UpdateViewInfo(struct FMinimalViewInfo& OutPOV, float DeltaTime)
{
	if (true || bAlwaysApplyModifiers)
	{
		//ApplyCameraModifiers(DeltaTime, OutPOV);
		for (int32 ModifierIdx = 0; ModifierIdx < ModifierList.Num(); ++ModifierIdx)
		{
			// Apply camera modification and output into DesiredCameraOffset/DesiredCameraRotation
			if ((ModifierList[ModifierIdx] != NULL) && !ModifierList[ModifierIdx]->IsDisabled())
			{
				// If ModifyCamera returns true, exit loop
				// Allows high priority things to dictate if they are
				// the last modifier to be applied
				if (ModifierList[ModifierIdx]->ModifyCamera(DeltaTime, OutPOV))
				{
					break;
				}
			}
		}
	}
}

void AOfflineCameraController::SetupViewport(const FMinimalViewInfo& InPOV)
{
	if (InspectorCamera)
	{
		InspectorCamera->SetActorLocation(POV.Location);
		InspectorCamera->SetActorRotation(POV.Rotation);
		if (UCameraComponent* CamComp = InspectorCamera->GetCameraComponent())
		{
			CamComp->SetFieldOfView(POV.FOV);
		}
	}
}

bool AOfflineCameraController::ShouldTickIfViewportsOnly() const
{
	return true;
}

FMinimalViewInfo AOfflineCameraController::GetCameraCachePOV() const
{
	FMinimalViewInfo OutPOV;
	OutPOV.Location = ThirdPlayerCamera->GetActorLocation();
	OutPOV.Rotation = ThirdPlayerCamera->GetActorRotation();

	return OutPOV;
}

float AOfflineCameraController::GetFOVAngle() const
{
	if (UCameraComponent* CamComp = ThirdPlayerCamera->GetCameraComponent())
	{
		return CamComp->FieldOfView;
	}
	
	return 90.f;
}
