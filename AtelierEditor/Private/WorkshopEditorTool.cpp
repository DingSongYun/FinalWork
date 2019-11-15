// Fill out your copyright notice in the Description page of Project Settings.

#include "WorkshopEditorTool.h"
//#include "AssetRegistryModule.h"
#include "Editor.h"


// Sets default values
AWorkshopEditorTool::AWorkshopEditorTool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AWorkshopEditorTool::~AWorkshopEditorTool()
{

}

// Called when the game starts or when spawned
void AWorkshopEditorTool::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWorkshopEditorTool::DestroyEditor()
{

}
