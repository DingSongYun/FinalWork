#include "PlacementCategory.h"

FPlacementCategory::FPlacementCategory(UPlaceableCategoryFactory* Factory)
{
    ensure(Factory && !Factory->GetCategoryName().IsNone());
    Name = Factory->GetCategoryName();
    bPlacementsShowInAll = Factory->ShowInAll();
    PlacementSet.Empty();

    Factory->CollectPlacements(PlacementSet);
}

FPlacementCategory::FPlacementCategory(const FName& InName)
    : Name(InName), bPlacementsShowInAll(false)
{
    PlacementSet.Empty();
}

void FPlacementCategory::AddPlacementObject(FPlacementItemPtr Obj)
{
    PlacementSet.AddUnique(Obj);
}