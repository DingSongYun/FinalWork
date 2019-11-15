#include "PlacementCharacter.h"
#include "Components/ChildActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "StageEditor/StageEditorProxy.h"
#include "Engine/World.h"
#include "LevelEditor.h"
#include "ModuleManager.h"
#include "DrawDebugHelpers.h"

APlacementCharacterSpawner::APlacementCharacterSpawner(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bShowHuntArea(true)
    , bShowCautionArea(true)
    , bShowPatrolArea(true)
{
    FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnActorSelectionChanged().AddUObject(this, &APlacementCharacterSpawner::OnActorSelectionChanged);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void APlacementCharacterSpawner::OnActorSelectionChanged(const TArray<UObject*>& NewSelection, bool bForceRefresh)
{
    for (UObject* Obj : NewSelection)
    {
        if (Obj == ChildCharacter)
        {
            GEditor->SelectActor(ChildCharacter, /*bSelected=*/ false, /*bNotify=*/ true, true, false);
            GEditor->SelectActor(this, /*bSelected=*/ true, /*bNotify=*/ true, true, true);
        }
    }
}

void APlacementCharacterSpawner::PostRegisterAllComponents()
{
    Super::PostRegisterAllComponents();
}

void APlacementCharacterSpawner::AssembleCharacter(uint32 CharacterId)
{
    ConfData.Id = CharacterId;
    PostAssembleCharacter();
}

void APlacementCharacterSpawner::PostAssembleCharacter()
{
    if (ChildCharacter == nullptr) CreateChildCharacter();

    ULuaStageEdProxy* EdProxy = FStageEditorProxy::GetLuaEdProxy();
    check(EdProxy);

    if (ACharacter* Character = GetChildCharacter())
    {
        EdProxy->AssembleCharacter(Character, ConfData.Id);

        for (UActorComponent* Component : Character->GetComponentsByClass(USkeletalMeshComponent::StaticClass()))
        {
            if (USkeletalMeshComponent* MeshComp = Cast<USkeletalMeshComponent>(Component))
            {
                MeshComp->SetUpdateAnimationInEditor(true);
            }
        }

        Character->SetActorRelativeLocation(FVector(0, 0, Character->GetDefaultHalfHeight()));
    }
}

void APlacementCharacterSpawner::BeginDestroy()
{
    Super::BeginDestroy();
    DestroyChildCharacter();
}

void APlacementCharacterSpawner::UnregisterAllComponents(bool bForReregister)
{
    Super::UnregisterAllComponents(bForReregister);
}

void APlacementCharacterSpawner::MarkComponentsAsPendingKill()
{
    Super::MarkComponentsAsPendingKill();
    DestroyChildCharacter();
}

void APlacementCharacterSpawner::CreateChildCharacter()
{
    // InnerCharacter = CreateDefaultSubobject<UChildActorComponent>("CharacterActor");
    // Kill spawned actor if we have one
    DestroyChildCharacter();
    // If we have a class to spawn.
    if(ChildCharacterClass != nullptr)
    {
        if(UWorld* World = GetWorld())
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            Params.bDeferConstruction = true;
            Params.bAllowDuringConstructionScript = true;
            Params.OverrideLevel = GetLevel();
            Params.ObjectFlags |= (RF_TextExportTransient | RF_NonPIEDuplicateTransient);
            if (!HasAllFlags(RF_Transactional))
            {
                Params.ObjectFlags &= ~RF_Transactional;
            }
            if (HasAllFlags(RF_Transient) || IsEditorOnly())
            {
                // If we are either transient or editor only, set our created actor to transient. We can't programatically set editor only on an actor so this is the best option
                Params.ObjectFlags |= RF_Transient;
            }


            FVector Location = GetActorLocation();
            FRotator Rotation = GetActorRotation();
            ChildCharacter = Cast<ACharacter>(World->SpawnActor(ChildCharacterClass, &Location, &Rotation, Params));
            ChildCharacter->FinishSpawning(GetActorTransform(), false, nullptr);
            ChildCharacter->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        }
    }
}

void APlacementCharacterSpawner::DestroyChildCharacter()
{
    if (ChildCharacter)
    {
        const bool bIsChildActorPendingKillOrUnreachable = ChildCharacter->IsPendingKillOrUnreachable();
        UWorld* World = ChildCharacter->GetWorld();
        if (!bIsChildActorPendingKillOrUnreachable)
        {
            World->DestroyActor(ChildCharacter);
        }
    }
    ChildCharacter = nullptr;
}

#define CHECK_PROP_NAME(PropName, MemberName) \
    PropName == GET_MEMBER_NAME_CHECKED(FConfData_Character, MemberName)
void APlacementCharacterSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (PropertyChangedEvent.Property != nullptr)
    {
        FName PropName = PropertyChangedEvent.Property->GetFName();
        if (CHECK_PROP_NAME(PropName, Id))
        {
            PostAssembleCharacter();
        }
        else if (CHECK_PROP_NAME(PropName, HuntArea))
        {
            ConfData.CautionArea = FMath::Max(ConfData.HuntArea + 1, ConfData.CautionArea);
        }
        else if (CHECK_PROP_NAME(PropName, CautionArea))
        {
            ConfData.HuntArea = FMath::Max(FMath::Min(ConfData.CautionArea - 1, ConfData.HuntArea), 0);
        }
    }
}

bool APlacementCharacterSpawner::CanEditChange(const UProperty* InProperty) const
{
    return true;
}

bool APlacementCharacterSpawner::ShouldTickIfViewportsOnly() const
{
    return true;
}

void APlacementCharacterSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    DrawDebug(0.03);
}

void APlacementCharacterSpawner::DrawDebug(float DeltaTime)
{
	if (!IsSelectedInEditor())
		return ;
    
	FTransform MyTransform = GetTransform();
	if (ConfData.PatrolPath.Num() > 1)
	{
		FVector WaypointLocation = ConfData.PatrolPath[0].GetWorldLocation(MyTransform);

		for (int32 WaypointIndex = 1; WaypointIndex < ConfData.PatrolPath.Num(); ++WaypointIndex)
		{
			const FVector NextWaypoint = ConfData.PatrolPath[WaypointIndex].GetWorldLocation(MyTransform);
			DrawDebugLine(GetWorld(), WaypointLocation, NextWaypoint, FColor::Red, false, DeltaTime);
			WaypointLocation = NextWaypoint;
		}
	}
	
	// Caution Area
	if (bShowCautionArea && ConfData.CautionArea > 0)
	{
		DrawDebugSphere(GetWorld(), MyTransform.GetLocation(), ConfData.CautionArea, 50, FColor::Blue, false, DeltaTime, 3.0f);
	}

	// Hunt Area
	if (bShowHuntArea && ConfData.HuntArea > 0)
	{
		DrawDebugSphere(GetWorld(), MyTransform.GetLocation(), ConfData.HuntArea, 50, FColor::Yellow, false, DeltaTime, 3.0f);
	}

    // Patrol Area
	if (bShowPatrolArea && ConfData.PatrolArea >0)
	{
		DrawDebugSphere(GetWorld(), MyTransform.GetLocation(), ConfData.PatrolArea, 50, FColor::Purple, false, DeltaTime, 3.0f);
	}
}

void APlacementCharacterSpawner::PostSerialize()
{
    AssembleCharacter( ConfData.Id );
}