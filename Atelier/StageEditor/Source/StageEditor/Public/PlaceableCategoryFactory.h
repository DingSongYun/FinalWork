// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-21

#pragma once

#include "CoreMinimal.h"
#include "IStagePlacement.h"
#include "PlaceableCategoryFactory.generated.h"

struct FPlacementItem;

UCLASS(abstract)
class STAGEEDITOR_API UPlaceableCategoryFactory : public UObject 
{
    GENERATED_BODY()

public:
    virtual FName GetCategoryName() const
    {
        return NAME_None;
    }

    virtual void CollectPlacements(TArray<TSharedPtr<FPlacementItem>>& OutSet) const
    {}

    virtual bool ShowInAll() const
    {
        return false;
    }
};
