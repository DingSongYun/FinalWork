// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-24

#pragma once

#include "CoreMinimal.h"
#include "PlaceableCategoryFactory.h"

#include "StagePlacementCategory.generated.h"

UCLASS()
class UBasicFactory : public UPlaceableCategoryFactory
{
    GENERATED_BODY()
public:
    FName GetCategoryName() const override
    {
        return FName("Basic");
    }
};

/*------------------------------------------------------
 配置在Npc表里的那些数据对应的场景物件
------------------------------------------------------*/
UCLASS(abstract)
class UTblConfiguredFactory : public UPlaceableCategoryFactory
{
    GENERATED_BODY()
protected:
    FName GetTableName() const;
    void GetTableData(TArray<class UGADataRow*>& OutData) const;
    virtual bool CanSpawnWithType(int32 InType) const { return false; }
    virtual FAssetData GetAsset() const { return FAssetData(); }
    virtual void CollectPlacements(TArray<TSharedPtr<FPlacementItem>>& OutSet) const override;
};

UCLASS()
class UCharacterFactory : public UTblConfiguredFactory
{
    GENERATED_BODY()
public:
    FName GetCategoryName() const override
    {
        return FName("Characters");
    }

    virtual bool CanSpawnWithType(int32 InType) const override
    {
        return  InType == 20           // 伙伴
            || InType == 10            // Npc
            || InType == 100           // 技能召唤的Npc
            ;
    }
    virtual FAssetData GetAsset() const override;
};

UCLASS()
class UMonsterFactory : public UTblConfiguredFactory
{
    GENERATED_BODY()
public:
    FName GetCategoryName() const override
    {
        return FName("Monsters");
    }

    virtual bool CanSpawnWithType(int32 InType) const override
    {
        return  InType == 60           // 普通魔物
            || InType == 70            // 精英魔物
            || InType == 80            // Boss魔物
            ;
    }

    virtual FAssetData GetAsset() const override;
};

UCLASS()
class UCollectionFactory : public UTblConfiguredFactory
{
    GENERATED_BODY()
public:
    FName GetCategoryName() const override
    {
        return FName("Collections");
    }

    virtual bool CanSpawnWithType(int32 InType) const override
    {
        return  InType == 120;
    }

    virtual FAssetData GetAsset() const override;
};

UCLASS()
class UGearFactory : public UTblConfiguredFactory
{
    GENERATED_BODY()
public:
    FName GetCategoryName() const override
    {
        return FName("Gears");
    }

    virtual bool CanSpawnWithType(int32 InType) const override
    {
        return InType == 30            // 陷阱 & 机关
            || InType == 50        // 阻挡物
            || InType == 90        // 可破坏的交互物
            || InType == 120       // 家具
            || InType == 130       // 角色身上的物品
            || InType == 140       // 宝箱
            ;
    }

    virtual FAssetData GetAsset() const override;
};