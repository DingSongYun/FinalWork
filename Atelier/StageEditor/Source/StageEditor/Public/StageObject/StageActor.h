// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-18

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IStagePlacement.h"

#include "StageActor.generated.h"

UCLASS()
class AStageActor : public AActor, public IStagePlacement
{
    GENERATED_UCLASS_BODY()
public:
};