// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EditorConfig.generated.h"

/**
 * 
 */
UCLASS(Config=Game)
class ATELIEREDITOR_API UEditorConfig : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config)
	FString DesignerDir;
};
