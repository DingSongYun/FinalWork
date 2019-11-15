// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-18

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "IStagePlacement.generated.h"

#define GENERATED_SERIALIZE_INFO(ConfData, Identifier) \
    void AssembleSerializeInfo(FStageArchive& Archive) override \
    { \
        Archive.Setup(ConfData.StaticStruct(), (uint8*)(&ConfData), Identifier); \
    }

UENUM(BlueprintType)
enum class EPlacementIdentifier: uint8
{
    PlayerStarts,
    Monsters,
    NPCs,
    Triggers,
    Placements,
};

USTRUCT(Blueprintable)
struct FStageArchive
{
    GENERATED_BODY()
public:
    void Setup(const UStruct* InScriptStruct, uint8* InData, EPlacementIdentifier InIdentifier = EPlacementIdentifier::Placements)
    {
        ScriptStruct = InScriptStruct;
        StructMemory = InData;
        Identifier = InIdentifier;
    }

    bool IsValid() const
    {
        return ScriptStruct.IsValid() && StructMemory;
    }

    TWeakObjectPtr<const UStruct> ScriptStruct;
    uint8* StructMemory;
    EPlacementIdentifier Identifier;
    FSoftClassPath ObjClass;
    FTransform Transform;
};

UINTERFACE(MinimalAPI)
class UStagePlacement : public UInterface
{
    GENERATED_UINTERFACE_BODY()
    // GENERATED_BODY()
};

class STAGEEDITOR_API IStagePlacement
{
    GENERATED_IINTERFACE_BODY()

public:
    IStagePlacement();

    virtual void AssembleSerializeInfo(FStageArchive& Archive) {}
    virtual void PostSerialize() {}
};
