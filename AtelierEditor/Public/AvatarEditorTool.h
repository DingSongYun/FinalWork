// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Model/CharacterPrefabModel.h"
#include "Common/GADataTable.h"
#include "AvatarEditorTool.generated.h"

UCLASS()
class ATELIEREDITOR_API AAvatarEditorTool : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAvatarEditorTool();
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, CallInEditor)
	void SetupEditorChar(int32 npcid);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnInit();

	FString PackagePath = FString("/Game/Data/DataTables/CharacterPrefabData");
	UPackage *Package = CreatePackage(nullptr, *PackagePath);
	UDataTable* DataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrNpcID = 0;

	UFUNCTION(BlueprintCallable)
	void LoadData(int32 CharacterID, FCharacterPrefabData& Data);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnLoadTransDataFinished(FTransform trans);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform CurrTransform;

	UFUNCTION(BlueprintCallable)
	void SaveData(FCharacterPrefabData Data);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACharacter* Char;

	FTransform LoadTransform;

	void SelectComp();

	UFUNCTION(BlueprintCallable)
	bool SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverWriting = false);

	UFUNCTION(BlueprintCallable)
	void SaveCharacterPrefab(FCharacterPrefabData Data);
};
