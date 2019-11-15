// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"

class ATELIEREDITOR_API EditorPlayerSettings : public UDeveloperSettings
{
public:
	static EditorPlayerSettings* Get();

	UPROPERTY(EditAnywhere, config, Category = Editing)
	FFilePath LastMapDataPath;
};
