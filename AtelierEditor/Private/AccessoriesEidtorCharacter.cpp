// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: I-RUNG.YOU
// Date: 2018-11-29

#include "AccessoriesEidtorCharacter.h"
#include "AssetRegistryModule.h"
#include "DataTables/Accessories.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"  
#include "ConstructorHelpers.h"
#include "Common/GADataTable.h"
#include "System/Project.h"
#include "Editor.h"


// Sets default values
AAccessoriesEidtorCharacter::AAccessoriesEidtorCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetVisibility(false);

	BodyComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Body"));
	BodyComponent->AttachToComponent(this->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
	BodyComponent->SetSkeletalMesh(BodyMesh);
	BodyComponent->SetRelativeLocation(FVector(0,0,-87));

	HeadComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Head"));
	HeadComponent->AttachToComponent(this->BodyComponent, FAttachmentTransformRules::KeepRelativeTransform, FName("HairMain"));
	HeadComponent->SetSkeletalMesh(HeadMesh);

	HairComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hair"));
	HairComponent->AttachToComponent(this->BodyComponent, FAttachmentTransformRules::KeepRelativeTransform, FName("HairMain"));

	AccessoriesComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Accessories"));
	AccessoriesComponent->AttachToComponent(this->HairComponent, FAttachmentTransformRules::KeepRelativeTransform);

	FAccessories row;
	FTransform CommonTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1));
	row.TransMap.Add(FName(TEXT("Common")), CommonTransform);
	FString AssetPath = FString("/Game/Data/DataTables/");
	if (LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/Accessories.Accessories'")))
	{
		DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/Accessories.Accessories'"));
		DataTable->RowStruct = row.StaticStruct();
		UE_LOG(LogAtelier, Warning, TEXT("Find"));
	}
	else {
		UE_LOG(LogAtelier, Warning, TEXT("Create"));
		DataTable = NewObject<UDataTable>(Package, UDataTable::StaticClass(), *FString("Accessories"), EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
		DataTable->RowStruct = row.StaticStruct();
		TMap<FName, FTransform> ConstructMap;
		DataTable->AddRow(FName(TEXT("Common")), row);
		//Create DataTable
		FAssetRegistryModule::AssetCreated(DataTable);
		DataTable->MarkPackageDirty();

		FString FilePath = FString::Printf(TEXT("%s%s%s"), *AssetPath, *FString("Accessories"), *FPackageName::GetAssetPackageExtension());
		bool bSuccess = UPackage::SavePackage(Package, DataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
		if (bSuccess)
		{
			UPackage::SavePackage(Package, DataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
		}
	}

}

void AAccessoriesEidtorCharacter::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	static const FName PROP_HAIR_MESH = "HairMesh";
	static const FName PROP_ACCESSORIES_MESH = "AccessoriesMesh";

	if (PropertyChangedEvent.Property != nullptr)
	{
		if (PropertyChangedEvent.Property->GetFName() == PROP_HAIR_MESH)
		{
			HairComponent->SetSkeletalMesh(HairMesh);
		}
		else if (PropertyChangedEvent.Property->GetFName() == PROP_ACCESSORIES_MESH)
		{
			AccessoriesComponent->SetStaticMesh(AccessoriesMesh);
		}
	}


}

// Called when the game starts or when spawned
void AAccessoriesEidtorCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}



void AAccessoriesEidtorCharacter::SetDataTable(FName AccessoriesName, FName HairName, bool isCommon)
{
	bool MapAdded = false;
	if (DataTable != nullptr)
	{
		TArray<FAccessories*> OutAllRows;
		FString Context; // Used in error reporting.
		DataTable->GetAllRows<FAccessories>(Context, OutAllRows); // Populates the array with all the data from the CSV.
		TArray<FName> key = DataTable->GetRowNames();
		if (key.Num()) {
			for (const FName val : key)
			{
				if (val == AccessoriesName)
				{
					if (isCommon) {
						UE_LOG(LogAtelier, Warning, TEXT("Add Common"));
						DataTable->FindRow<FAccessories>(val, Context)->TransMap.Add(FName(TEXT("Common")), AccessoriesComponent->GetRelativeTransform());
						MapAdded = true;
					}
					else {
						UE_LOG(LogAtelier, Warning, TEXT("Add Map"));
						DataTable->FindRow<FAccessories>(val, Context)->TransMap.Add(HairName, AccessoriesComponent->GetRelativeTransform());
						MapAdded = true;
					}
				}
			}
		}
		if (MapAdded)
		{
			MapAdded = false;
		}
		else
		{
			FAccessories row;
			if (isCommon)
			{
				row.TransMap.Add(FName(TEXT("Common")), AccessoriesComponent->GetRelativeTransform());
			}
			else
			{
				FTransform CommonTranform = FTransform(FRotator(0,0,0),FVector(0,0,0),FVector(1,1,1));
				row.TransMap.Add(FName(TEXT("Common")), CommonTranform);
				row.TransMap.Add(HairName, AccessoriesComponent->GetRelativeTransform());
			}

			DataTable->AddRow(AccessoriesName, row);
			DataTable->MarkPackageDirty();
		}

		FString AssetPath = FString("D:/Game/atelier/client-ue4/Content/Data/DataTables/");
		FString FilePath = FString::Printf(TEXT("%s%s%s"), *AssetPath, *FString("Accessories"), *FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(Package, DataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
	}
}


void AAccessoriesEidtorCharacter::SetHairComponentMesh(USkeletalMesh * newSkeleton)
{
	HairMesh = newSkeleton;
	HairComponent->SetSkeletalMesh(newSkeleton);
	SelectAccessoriesComponent();
}

void AAccessoriesEidtorCharacter::SetAccessoriesComponentMesh(UStaticMesh* newStatic)
{
	AccessoriesMesh = newStatic;
	AccessoriesComponent->SetStaticMesh(newStatic);
	SelectAccessoriesComponent();
}

void AAccessoriesEidtorCharacter::SelectAccessoriesComponent()
{
	LoadData();
	GEditor->SelectNone(true,true);
	GEditor->SelectActor(this,true,true);
	GEditor->SelectComponent(AccessoriesComponent,true,true);
}


void AAccessoriesEidtorCharacter::LoadData()
{
	if (AccessoriesMesh) {
		FString Context;
		FAccessories row;
		FTransform LoadTransform;
		DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/Accessories.Accessories'"));
		if (DataTable->GetRowNames().Num())
		{
			TArray<FName> RowNames = DataTable->GetRowNames();
			if (RowNames.Contains(*AccessoriesMesh->GetName())) {
				int32 TransMapLength = DataTable->FindRow<FAccessories>(*AccessoriesMesh->GetName(), Context)->TransMap.Num();
				if (TransMapLength)
				{
					TMap<FName, FTransform> map = DataTable->FindRow<FAccessories>(*AccessoriesMesh->GetName(), Context)->TransMap;
					if (map.Contains(*HairMesh->GetName())) {
						LoadTransform = *DataTable->FindRow<FAccessories>(*AccessoriesMesh->GetName(), Context)->TransMap.Find(*HairMesh->GetName());
						UE_LOG(LogAtelier, Warning, TEXT("Transform Change to Map Value"));
					}
					else
					{
						LoadTransform = *DataTable->FindRow<FAccessories>(*AccessoriesMesh->GetName(), Context)->TransMap.Find(FName(TEXT("Common")));
						UE_LOG(LogAtelier, Warning, TEXT("Transform Change to Common"));

					}
				}
			}
			else {
				LoadTransform = *DataTable->FindRow<FAccessories>(FName(TEXT("Common")), Context)->TransMap.Find(FName(TEXT("Common")));
				UE_LOG(LogAtelier, Warning, TEXT("Transform Change to Common"));
			}
		}
		AccessoriesComponent->SetRelativeTransform(LoadTransform);
	}
};

// Called every frame
void AAccessoriesEidtorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
