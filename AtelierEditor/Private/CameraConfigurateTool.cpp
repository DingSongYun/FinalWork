#include "CameraConfigurateTool.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

ACameraConfigurateTool::ACameraConfigurateTool(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

ACameraConfigurateTool::~ACameraConfigurateTool()
{
}

void ACameraConfigurateTool::BeginPlay()
{
	GEngine->OnActorMoved().AddUObject(this, &ACameraConfigurateTool::OnActorMove);
}

void ACameraConfigurateTool::Destroyed()
{
	GEngine->OnActorMoved().RemoveAll(this);
	if (ActorLocked)
	{
		ActorLocked->GetRootComponent()->TransformUpdated.RemoveAll(this);
	}
}

bool ACameraConfigurateTool::IsEditMode() const 
{
	return Mode == EEditMode::Edit_FocusMode || Mode == EEditMode::Edit_FreeMode;
}

FRotator LookRotationToTarget(const FVector& FromLocation, const AActor* Target)
{
	return UKismetMathLibrary::FindLookAtRotation(FromLocation, Target->GetActorLocation());
}

bool ACameraConfigurateTool::CanEditChange(const UProperty* InProperty) const 
{
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, CameraToParam)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, bCameraStartUseCurrent)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, bNeedMoveMeToTarget)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, bInterpolateByDistance)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, Duration)
	)
	{
		return IsEditMode();
	}

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, CameraFromParam))
	{
		return IsEditMode() && !bCameraStartUseCurrent;
	}

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, CameraToParamAdd))
	{
		return IsEditMode() && bInterpolateByDistance;
	}

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, BaseDistance))
	{
		return IsEditMode() && bInterpolateByDistance;
	}

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, DistanceMeToTarget))
	{
		return IsEditMode() && bNeedMoveMeToTarget;
	}
	return Super::CanEditChange(InProperty);
}

void ACameraConfigurateTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if(UProperty* InProperty = PropertyChangedEvent.MemberProperty)
	{
		if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, CameraFromParam))
		{
			CameraFromParam.FilledToCachedParam(CamFromTarget, Mode);
			FlushLockedActor();
			return ;
		}
		else if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, CameraToParam))
		{
			CameraToParam.FilledToCachedParam(CamFocusTarget, Mode);
			FlushLockedActor();
			return ;
		}
		else if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, CameraToParamAdd))
		{
			CameraToParamAdd.FilledToCachedParam(CamFocusTarget, Mode);
			FlushLockedActor();
			return ;
		}
	}

	if(UProperty* InProperty = PropertyChangedEvent.Property)
	{
		if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, CameraID))
		{
			return PostLoadCameraConfiguration(CameraID);
		}
		if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, CamFocusTarget))
		{
			return RefreshMultiCoordParam();
		}
		else if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ACameraConfigurateTool, Mode))
		{
			return OnEditModeChanged(Mode);
		}
		
	}
	return Super::PostEditChangeProperty(PropertyChangedEvent);
}

//void ACameraConfigurateTool::PreEditChange( FEditPropertyChain& PropertyAboutToChange )
//{
//	//Super::PreEditChange(PropertyAboutToChange);
//}

void ACameraConfigurateTool::PostLoadCameraConfiguration(FName inCameraID)
{
	BP_PostLoadCameraConfiguration(inCameraID);

	RefreshMultiCoordParam();
}

bool ACameraConfigurateTool::IsPlaying()
{
	UWorld* World = GetWorld();
	return World->WorldType != EWorldType::Editor;
}

void ACameraConfigurateTool::OnEditModeChanged(EEditMode inMode)
{
	RefreshMultiCoordParam();
}

void ACameraConfigurateTool::RefreshMultiCoordParam()
{
	CameraFromParam.RecalculateFromCachedParam(CamFromTarget, Mode);
	CameraToParam.RecalculateFromCachedParam(CamFocusTarget, Mode);
	CameraToParamAdd.RecalculateFromCachedParam(CamFocusTarget, Mode);
}

void ACameraConfigurateTool::SetActorLocked(class AActor* Actor)
{
	AActor* PreActor = ActorLocked;
	if (PreActor)
	{
		PreActor->GetRootComponent()->TransformUpdated.RemoveAll(this);
	}
	ActorLocked = Actor;
	if (ActorLocked)
	{
		ActorLocked->GetRootComponent()->TransformUpdated.AddUObject(this, &ACameraConfigurateTool::OnActorLockedMove);
	}
}

void ACameraConfigurateTool::FlushLockedActor()
{
	//return ;
	if (ActorLocked == nullptr)
	{
		return ;
	}

	const FTransform& Transfrom = CamFocusTarget->GetActorTransform();
	FVector WLocation = Transfrom.TransformPosition(CameraToParam.CachedParam.Location);
	FRotator WRotation = LookRotationToTarget(WLocation, CamFocusTarget) + CameraToParam.CachedParam.Rotation;
	ActorLocked->SetActorLocation(WLocation);
	ActorLocked->SetActorRotation(WRotation);
	//UE_LOG(LogAtelier, Warning, TEXT("Flush Locked Actor: from(%s, %s) => to(%s, %s)"), 
	//	*CameraToParam.CachedParam.Location.ToString(), *CameraToParam.CachedParam.Rotation.ToString(),
	//	*WLocation.ToString(), *WRotation.ToString()
	//	);
}

void ACameraConfigurateTool::SyncLockedActor()
{
	//return ;
	if (Mode == EEditMode::Preview)
	{
		return ;
	}

	if (ActorLocked == nullptr || CamFocusTarget == nullptr)
	{
		return ;
	}

	FVector WLocation = ActorLocked->GetActorLocation();
	FRotator WRotation = ActorLocked->GetActorRotation();

	const FTransform& Transfrom = CamFocusTarget->GetActorTransform();
	CameraToParam.CachedParam.Location = Transfrom.InverseTransformPosition(WLocation);
	CameraToParam.CachedParam.Rotation = WRotation - LookRotationToTarget(WLocation, CamFocusTarget);


	//UE_LOG(LogAtelier, Warning, TEXT("Sync Locked Actor: from(%s, %s) => to(%s, %s)"), 
	//	*WLocation.ToString(), *WRotation.ToString(),
	//	*CameraToParam.CachedParam.Location.ToString(), *CameraToParam.CachedParam.Rotation.ToString()
	//	);

	CameraToParam.RecalculateFromCachedParam(CamFocusTarget, Mode);
}

void FMultiCoordPOVParam::RecalculateFromCachedParam(class AActor* Target, EEditMode Mode)
{
	if (Target == nullptr)
	{
		Location = CachedParam.Location;
		Rotation = CachedParam.Rotation;
		FOV = CachedParam.FOV;
		return ;
	}

	switch (Mode)
	{
	case EEditMode::Preview:
	case EEditMode::Edit_FocusMode:
	{
		Location = CachedParam.Location;
		Rotation = CachedParam.Rotation;
		FOV = CachedParam.FOV;
	}
		break;
	case EEditMode::Edit_FreeMode:
	{
		const FTransform& Transfrom = Target->GetActorTransform();

		Location = Transfrom.TransformPosition(CachedParam.Location);
		Rotation = LookRotationToTarget(Location, Target) + CachedParam.Rotation;
		FOV = CachedParam.FOV;
	}
		break;
	}
}

void FMultiCoordPOVParam::FilledToCachedParam(class AActor* Target, EEditMode Mode)
{
	if (Target == nullptr)
	{
		CachedParam.Location = Location;
		CachedParam.Rotation = Rotation;
		CachedParam.FOV = FOV;
		return ;
	}
	
	switch (Mode)
	{
	case EEditMode::Preview:
	case EEditMode::Edit_FocusMode:
	{
		CachedParam.Location = Location;
		CachedParam.Rotation = Rotation;
		CachedParam.FOV = FOV;
	}
		break;
	case EEditMode::Edit_FreeMode:
	{
		const FTransform& Transfrom = Target->GetActorTransform();

		CachedParam.Location = Transfrom.InverseTransformPosition(Location);
		//CachedParam.Rotation = Transfrom.InverseTransformRotation(Rotation.Quaternion()).Rotator();
		CachedParam.Rotation = Rotation - LookRotationToTarget(Location, Target);
		CachedParam.FOV = FOV;
	}
		break;
	}
}
	
void ACameraConfigurateTool::OnActorMove(AActor* InActor)
{
	if (InActor)
	{
		UE_LOG(LogAtelier, Warning, TEXT("OnActorMove: %s"), *InActor->GetActorLabel());
		if (InActor == ActorLocked)
		{
			if (Mode != EEditMode::Preview)
			{
				SyncLockedActor();
			}
		}
	}
}

void ACameraConfigurateTool::OnActorLockedMove(USceneComponent* InRootComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	if (Mode != EEditMode::Preview)
	{
		SyncLockedActor();
	}
}