// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-11-14
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "StageConfigStruct.generated.h"

USTRUCT(BlueprintType)
struct STAGEEDITOR_API FStageConfig
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    FString FilePath;
};