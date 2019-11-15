// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-19

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "ActionSkill.h"
#include "Engine/UserDefinedStruct.h"
#include "Kismet2/StructureEditorUtils.h"
#include "EdGraphSchema_K2.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "AssetTypeCategories.h"
#include "ActionSkillEditorModule.h"

#include "ASTypeFactory.generated.h"

UCLASS(collapsecategories, hidecategories=Object, editinlinenew, config=Editor, abstract, transient)
class UASTypeFactoryBase : public UFactory
{
	GENERATED_BODY()
public:
    UASTypeFactoryBase(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
    {
        bCreateNew = true;
        bEditAfterNew = true;
    }

    bool ShouldShowInNewMenu() const override { return true; }

    uint32 GetMenuCategories() const override { return IActionSkillEditorModule::GetAdvanceAssetTypeCategory(); }

    template <typename T>
    UObject* FactoryCreateNewInner(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
    {
        T* Struct = NULL;

        Struct = NewObject<T>(InParent, Name, Flags);
        check(Struct);
        Struct->EditorData = NewObject<UUserDefinedStructEditorData>(Struct, NAME_None, RF_Transactional);
        check(Struct->EditorData);

        Struct->Guid = FGuid::NewGuid();
        Struct->SetMetaData(TEXT("BlueprintType"), TEXT("true"));
        Struct->SetMetaData(TEXT("ActionSkillNotify"), TEXT("true"));
        Struct->Bind();
        Struct->StaticLink(true);
        Struct->Status = UDSS_Error;

        // Default Parms
        {
            FStructureEditorUtils::AddVariable(Struct, FEdGraphPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
        }
        return Struct;
    }

    // FText GetDisplayName() const override { return FText::FromString(TEXT("None")); }
};

UCLASS()
class UActionEventArgsStructFactory : public UASTypeFactoryBase
{
	GENERATED_BODY()
public:

    UActionEventArgsStructFactory(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
    {
        SupportedClass = UActionEventArgsStruct::StaticClass();
    }

    UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
    {
        return FactoryCreateNewInner<UActionEventArgsStruct>(Class, InParent, Name, Flags, Context, Warn);
    }

    FText GetDisplayName() const override { return FText::FromString(TEXT("Event Args Struct")); }

};

UCLASS()
class UActionEventStructTypeFactory : public UASTypeFactoryBase
{
	GENERATED_BODY()
public:

    UActionEventStructTypeFactory(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
    {
        SupportedClass = UActionEventStructType::StaticClass();
    }

    UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
    {
        return FactoryCreateNewInner<UActionEventStructType>(Class, InParent, Name, Flags, Context, Warn);
    }

    FText GetDisplayName() const override { return FText::FromString(TEXT("SkillEventStruct")); }

};