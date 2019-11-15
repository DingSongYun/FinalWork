// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: I-RUNG.YOU
// Date: 2018-11-29

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "AccessoriesEidtorCharacter.generated.h"


UCLASS(BlueprintType)
class ATELIEREDITOR_API AAccessoriesEidtorCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAccessoriesEidtorCharacter();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	


	USkeletalMesh* HeadMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("SkeletalMesh'/Game/Assets/_Art/_Character/_Common/Facial/Head/Ciruru_morpher.Ciruru_morpher'"));
	USkeletalMesh* BodyMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("SkeletalMesh'/Game/Assets/_Art/_Character/Heroine/Suit/PlayerSkin_WizardJr/Model/PlayerSkin_WizardJr_BKH.PlayerSkin_WizardJr_BKH'"));


	UPROPERTY(Category = CharacterDefault, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* BodyComponent;
	UPROPERTY(Category = CharacterDefault, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HeadComponent;
	UPROPERTY(Category = Mesh, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", DisplayName = "头发"))
	USkeletalMeshComponent* HairComponent;
	UPROPERTY(Category = Mesh, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", DisplayName = "头饰"))
	UStaticMeshComponent* AccessoriesComponent;


public:
	FString PackagePath = FString("/Game/Data/DataTables/Accessories");
	UPackage *Package = CreatePackage(nullptr, *PackagePath);
	UDataTable* DataTable;

	UFUNCTION(BlueprintCallable, Category = "Data")
	void SetDataTable(FName AccessoriesName, FName HairName, bool isCommon);

	UPROPERTY(Category = AccessoriesSetup, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", DisplayName = "头发模型资源"))
	USkeletalMesh* HairMesh;

	UPROPERTY(Category = AccessoriesSetup, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", DisplayName = "头饰模型资源"))
	UStaticMesh* AccessoriesMesh;

	void SetHairComponentMesh(USkeletalMesh* newSkeleton);
	void SetAccessoriesComponentMesh(UStaticMesh* newStatic);
	void SelectAccessoriesComponent();
	void LoadData();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};


