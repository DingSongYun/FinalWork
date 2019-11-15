// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-16

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "SoftLevelPtr.generated.h"

USTRUCT(BlueprintType)
struct FSoftLevelRef
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<UWorld> LevelRef;
};

struct FSoftLevelPtr : public TSoftObjectPtr<UWorld>
{
};