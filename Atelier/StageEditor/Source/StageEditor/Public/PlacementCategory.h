// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-21

#pragma once
#include "PlaceableCategoryFactory.h"

struct FPlacementItem
{
public:
	FPlacementItem(UActorFactory* InFactory, const FAssetData& InAssetData)
		: Factory(InFactory)
		, AssetData(InAssetData)
	{
		AutoSetDisplayName();
	}

	void AutoSetDisplayName()
	{
		const bool bIsClass = AssetData.GetClass() == UClass::StaticClass();
		const bool bIsActor = bIsClass ? CastChecked<UClass>(AssetData.GetAsset())->IsChildOf(AActor::StaticClass()) : false;

		if (bIsActor)
		{
			AActor* DefaultActor = CastChecked<AActor>(CastChecked<UClass>(AssetData.GetAsset())->ClassDefaultObject);
			DisplayName = FText::FromString(FName::NameToDisplayString(DefaultActor->GetClass()->GetName(), false));
		}
		else if (bIsClass)
		{
			DisplayName = FText::FromString(FName::NameToDisplayString(AssetData.AssetName.ToString(), false));
		}
		else
		{
			DisplayName = FText::FromName(AssetData.AssetName);
		}
	}

	/** The factory used to create an instance of this placeable item */
	UActorFactory* Factory;

	/** Asset data pertaining to the class */
	FAssetData AssetData;

	/** This item's display name */
	FText DisplayName;
};

typedef TSharedPtr<FPlacementItem> FPlacementItemPtr;
typedef TArray<FPlacementItemPtr> FPlacementSet;

class FPlacementCategory
{
public:
    FPlacementCategory() {}
    FPlacementCategory(class UPlaceableCategoryFactory* Factory);
    FPlacementCategory(const FName& Name);
    ~FPlacementCategory() {}
    void AddPlacementObject(FPlacementItemPtr Obj);

public:
    FName           Name;
    FPlacementSet   PlacementSet;
    bool            bPlacementsShowInAll;   // 本Category下的物件是否显示在All的category下
};