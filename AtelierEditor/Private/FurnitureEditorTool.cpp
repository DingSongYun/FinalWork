// Fill out your copyright notice in the Description page of Project Settings.

#include "FurnitureEditorTool.h"
//#include "AssetRegistryModule.h"
#include "Editor.h"
#include "LuaBlueprintLibrary.h"
#include "EditorStaticLibrary.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "UObjectGlobals.h"
#include "ConstructorHelpers.h"


// Sets default values
AFurnitureEditorTool::AFurnitureEditorTool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AFurnitureEditorTool::~AFurnitureEditorTool()
{

}

// Called when the game starts or when spawned
void AFurnitureEditorTool::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFurnitureEditorTool::DestroyEditor()
{

}

AStaticMeshActor* AFurnitureEditorTool::GenGridVolume(AActor* Furniture, USkeletalMesh* mesh)
{
	GridVolumes.Empty();
	FBox box = mesh->GetBounds().GetBox();
	const FRotator Rotation = FRotator(0, 0, 0);
	FActorSpawnParameters params;
	AStaticMeshActor* aBox = GetWorld()->SpawnActor<AStaticMeshActor>(box.GetCenter(), Rotation, params);
	UStaticMeshComponent* comp = aBox->GetStaticMeshComponent();
	UStaticMesh* boxMesh = LoadObject<UStaticMesh>(nullptr, UTF8_TO_TCHAR("/Game/SoftwareOcculusion/Meshes/Occulusion_cube"));
	if (nullptr != boxMesh)
	{
		comp->SetStaticMesh(boxMesh);
		AActor* owner = comp->GetOwner();
		if (nullptr == owner)
		{
			aBox->SetActorLocation(box.GetCenter());
		}
	}
	aBox->SetActorLocation(box.GetCenter());
	aBox->SetActorScale3D(0.01*box.GetSize());
	//GridVolumes.Add(MakeShareable(aBox));
	return aBox;
}
