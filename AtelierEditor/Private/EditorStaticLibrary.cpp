#include "EditorStaticLibrary.h"
#include "LevelEditorViewport.h"
#include "Editor.h"

UEditorEngine* UEditorStaticLibrary::GetEditorEngine()
{
	return GEditor;
}

UWorld* UEditorStaticLibrary::GetEditorWorld()
{
	return GEditor->GetEditorWorldContext().World();
}

void UEditorStaticLibrary::PossessViewportToActor(AActor* InActor)
{
	if (InActor == nullptr)
		return ;

	for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
	{
		if ((LevelVC == nullptr) || !LevelVC->IsPerspective() || !LevelVC->AllowsCinematicControl())
		{
			continue;
		}

			//UpdatePreviewLevelViewportClientFromCameraCut(*LevelVC, CameraObject, bJumpCut);
			LockViewportToActor(LevelVC, InActor);
	}
}


void UEditorStaticLibrary::UnpossessViewportFromActor(AActor* InActor)
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

void UEditorStaticLibrary::LockViewportToActor(FLevelEditorViewportClient* InViewportClient, AActor* InActor)
{
	InViewportClient->SetViewLocation(InActor->GetActorLocation());
	InViewportClient->SetViewRotation(InActor->GetActorRotation());

	if (UCameraComponent* CameraComp = InActor->FindComponentByClass<UCameraComponent>())
	{
		InViewportClient->RemoveCameraRoll();
		InViewportClient->AspectRatio = CameraComp->AspectRatio;
		InViewportClient->ViewFOV = CameraComp->FieldOfView;
		InViewportClient->UpdateViewForLockedActor();
	}

	InViewportClient->SetActorLock(InActor);
}

void UEditorStaticLibrary::SetViewportLocation(const FVector& Location)
{
	if (GCurrentLevelEditingViewportClient)
	{
		return GCurrentLevelEditingViewportClient->SetViewLocation(Location);
	}
}

void UEditorStaticLibrary::SetViewportRotation(const FRotator& Rotation)
{
	if (GCurrentLevelEditingViewportClient)
	{
		GCurrentLevelEditingViewportClient->SetViewRotation(Rotation);
	}
}

void UEditorStaticLibrary::SetViewportFOV(float FOV)
{
	if (GCurrentLevelEditingViewportClient)
	{
		GCurrentLevelEditingViewportClient->ViewFOV = FOV;
	}
}

FVector UEditorStaticLibrary::GetViewportLocation(const FVector& Location)
{
	check(GCurrentLevelEditingViewportClient)
	return GCurrentLevelEditingViewportClient->GetViewLocation();
}

FRotator UEditorStaticLibrary::GetViewportRotation(const FRotator& Rotation)
{
	check(GCurrentLevelEditingViewportClient)
	return GCurrentLevelEditingViewportClient->GetViewRotation();
}

float UEditorStaticLibrary::GetViewportFOV(float fov)
{
	check(GCurrentLevelEditingViewportClient)
	return GCurrentLevelEditingViewportClient->ViewFOV;
}
