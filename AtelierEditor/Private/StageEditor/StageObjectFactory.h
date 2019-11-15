// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-24

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "StageObjects/PlacementCharacter.h"

#include "StageObjectFactory.generated.h"

UCLASS(abstract)
class UStageActorFactory : public UActorFactory
{
    GENERATED_BODY()
};

UCLASS()
class USActorFactoryCharacter : public UStageActorFactory
{
    GENERATED_UCLASS_BODY()
public:
    void SetCharacterId(uint32 CfgId) { CharacterCfgId = CfgId; }

    void PostSpawnActor(UObject* Asset, AActor* NewActor) override
    {
        UE_LOG(LogTemp, Warning, TEXT("PostSpawnActor: %s"), *NewActor->GetName());
        APlacementCharacterSpawner* Spawner = Cast<APlacementCharacterSpawner>(NewActor);
        check(Spawner);
        Spawner->AssembleCharacter(CharacterCfgId);
    }

    virtual bool CanCreateActorFrom( const FAssetData& AssetData, FText& OutErrorMsg ) override
    {
        return true;
        /*
        if (AssetData.GetClass()->IsChildOf(APlacementCharacterSpawner::StaticClass()))
        {
            return true;
        }

        return Super::CanCreateActorFrom(AssetData, OutErrorMsg);
        */
    }

    virtual AActor* GetDefaultActor( const FAssetData& AssetData )
    {
        if ( !AssetData.IsValid())
        {
            return NULL;
        }

        if (AssetData.GetClass()->IsChildOf( UBlueprint::StaticClass()))
        {
            const FString GeneratedClassPath = AssetData.GetTagValueRef<FString>(FBlueprintTags::GeneratedClassPath);
            if ( GeneratedClassPath.IsEmpty() )
            {
                return NULL;
            }

            UClass* GeneratedClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), NULL, *GeneratedClassPath, NULL, LOAD_NoWarn, NULL));

            if ( GeneratedClass == NULL )
            {
                return NULL;
            }

            return GeneratedClass->GetDefaultObject<AActor>();
        }

        return Super::GetDefaultActor(AssetData);
    }

private:
    uint32 CharacterCfgId;
};