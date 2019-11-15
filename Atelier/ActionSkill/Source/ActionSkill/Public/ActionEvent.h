// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding

#pragma once
#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "UserStructOnScope.h"
#include "Engine/UserDefinedStruct.h"
#include "ActionEvent.generated.h"

class ISerializableTableBase
{
public:
    void DoImport(const FString& SrcString, TArray<FString> Errors);
    FString ExportAsString();
};

/*********************************************************************/
// UActionEventStructType
/*********************************************************************/
UCLASS()
class ACTIONSKILL_API UActionEventStructType : public UUserDefinedStruct
{
    GENERATED_BODY()
public:
};

/*********************************************************************/
// UActionEventArgsStructType
/*********************************************************************/
UCLASS()
class ACTIONSKILL_API UActionEventArgsStruct : public UUserDefinedStruct
{
    GENERATED_BODY()
public:
};

#define SET_PROP_VALUE(Prop, InValue) \
	Prop->SetPropertyValue(Prop->ContainerPtrToValuePtr<void>(InnerStruct->GetStructMemory()), InValue)

/*********************************************************************/
// FActionEvent
/*********************************************************************/
USTRUCT(BlueprintType)
struct ACTIONSKILL_API FActionEvent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    int32 Id;

    UPROPERTY(EditAnywhere)
    FName Name;

    UPROPERTY(EditAnywhere)
    FUserStructOnScope Params;

    UPROPERTY()
    TArray<FUserStructOnScope> SubEvents;

    void ImportInnerUserStruct() = delete;
#if WITH_EDITOR
public:
	void SetId(int32 InId);

    void MarkDirty(bool bIsDirty, UProperty* DirtyProperty = nullptr);
    FORCEINLINE bool IsDirty() { return bDirty; }
    void ExportInnerUserStruct() = delete;

	DECLARE_DELEGATE_OneParam(FOnEventDataDirty, bool /* bIsDirty */);
	FOnEventDataDirty OnEventDataDirty;
	DECLARE_DELEGATE_OneParam(FOnEventIdChanged, FActionEvent& /* ActionEvent Point */);
	FOnEventIdChanged OnEventIdChanged;

private:
	//TSharedPtr<FStructOnScope> InnerStruct;
	//TMap<FName, UProperty*> PropDict;
    bool bDirty;
#endif
};

typedef TSharedPtr<FActionEvent> FActionEventPtr;
typedef TMap<int32, FActionEventPtr>::TIterator EventIterator;
typedef TMap<int32, FActionEventPtr>::TConstIterator EventConstIterator;

/*********************************************************************/
// UActionEventTable
/*********************************************************************/
UCLASS()
class ACTIONSKILL_API UActionEventTable : public UObject
{
    GENERATED_BODY()

    friend class FActionEventTableCSVExporter;
    friend class FActionEventTableCSVImporter;
    friend class FActionSkillModule;

public:
    UActionEventTable(const FObjectInitializer& ObjectInitializer);
    EventIterator CreateIterator() { return EventIterator(ActionEvents); }
    EventConstIterator CreateConstIterator() const { return EventConstIterator(ActionEvents); }
    void EmptyTable();
#if WITH_EDITOR
    FActionEventPtr NewEvent();
    void DeleteEvent(int32 Id);
    /** Callback when save table */
    void OnSaveData();

	void OnEventIdChanged(FActionEvent& InEvent);
    void UpdateNextValideId(int32 NewId);
#endif // WITH_EDITOR
private:
    /** Inner data container */
    TMap<int32, FActionEventPtr> ActionEvents;
};

USTRUCT(BlueprintType)
struct ACTIONSKILL_API FEffectVariable
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FString ValueType = "SkillLvUp";

	UPROPERTY(EditAnywhere)
	FUserStructOnScope Params;

	void ImportInnerUserStruct() = delete;
};