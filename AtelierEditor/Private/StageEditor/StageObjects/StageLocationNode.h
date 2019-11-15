// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-24
#pragma once

#include "CoreMinimal.h"
#include "StageLocationNode.generated.h"

USTRUCT(BlueprintType)
struct FStageLocationNode
{
	GENERATED_USTRUCT_BODY()

public:
	// You must be very careful that this position is a local location in owner coordinate
	UPROPERTY(EditAnywhere, Category=Default, BlueprintReadWrite, meta=(MakeEditWidget=""))
	FVector Location;

	FStageLocationNode()
		: Location(0, -50, 0)
	{}

	FStageLocationNode(const FVector& Position)
		: Location(Position)
	{}

	FVector GetWorldLocation(const FTransform& Transform)
	{
		return Transform.TransformPosition(Location);
	}
};