// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-19

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "UObject/StructOnScope.h"
#include "ActionSkillTypes.h"
#include "ActionSkill.generated.h"
//#include "LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogActionSkill, Log, All)

/*********************************************************************/
// FActionIterator
// 一个简单的FScriptArray的迭代器代理来做简单Action的
/*********************************************************************/
class FActionIterator
{
public:
    FActionIterator(FScriptArray* RawArray, int32 InElementSize, uint32 StartIndex = 0)
    {
        check(RawArray);
        Array = RawArray;
        ElementSize = InElementSize;
        Index = StartIndex;
    }

    FActionIterator& operator++()
    {
        ++ Index;
        return *this;
    }
    
    FActionIterator operator++(int)
    {
        FActionIterator Tmp(*this);
        ++ Index;
        return Tmp;
    }

    FActionIterator& operator--()
    {
        -- Index;
        return *this;
    }

    FActionIterator operator--(int)
    {
        FActionIterator Tmp(*this);
        -- Index;
        return Tmp;
    }
    
    uint32 GetIndex() const
    {
        return Index;
    }

    FAction& operator* () const
    {
        return *(FAction*) GetItemRawPtr();
    }

    FAction* operator-> () const
    {
        return (FAction*) GetItemRawPtr();
    }

    FORCEINLINE uint8* GetItemRawPtr() const
    {
        return (uint8*)Array->GetData() + Index * ElementSize;
    }

    FORCEINLINE explicit operator bool() const
    {
        return Array->IsValidIndex(Index);
    }

    void RemoveCurrent()
    {
        Array->Remove(Index, 1, ElementSize);
        Index--;
    }

    FORCEINLINE friend bool operator==(const FActionIterator& Lhs, const FActionIterator& Rhs) { return &Lhs.Array == &Rhs.Array && Lhs.Index == Rhs.Index; }
    FORCEINLINE friend bool operator!=(const FActionIterator& Lhs, const FActionIterator& Rhs) { return &Lhs.Array != &Rhs.Array || Lhs.Index != Rhs.Index; }

private:
    FScriptArray* Array;
    uint32 Index;
    int32 ElementSize;
};

/*********************************************************************/
// FActionSkillScope
// FActionSkill的代理，Skill的定义在c++中是不可见的,
// 用此来封装基础操作
/*********************************************************************/
USTRUCT()
struct ACTIONSKILL_API FActionSkillScope
{
    GENERATED_BODY()
public:
    void Initialize(UScriptStruct& SkillStruct);
    FORCEINLINE TSharedPtr<FStructOnScope> GetData() { return InnerStruct; }

    UProperty* GetSkillPropertyByName(const FName& InName);
    FString GetName();
    int32 GetId();
    FActionIterator CreateActionIterator();

	float GetSkillActionStartPos(int32 Index);
	float GetSkillActionLength(int32 Index);

    /** 计算技能长度(单位: 秒) */
    float CalculateSkillLength();

    /** 计算技能长度(单位: 帧数) */
    float CalculateSkillFrame();
private:
    /** Struct store skill data */
    TSharedPtr<FStructOnScope> InnerStruct;
    /** Cached prop map for fast find prop by name */
    TMap<FName, UProperty*> PropDict;

#if WITH_EDITOR
public:
    void SetId(int32 InId);
    void SetName(const FString& InName);
    FAction& NewAction();
    void RemoveAction(FAction& InAction);
    void MarkDirty(bool bIsDirty, UProperty* DirtyProperty = nullptr);
    FORCEINLINE bool IsDirty() { return bDirty; }

    DECLARE_DELEGATE_OneParam(FOnSkillDataDirty, bool /* bIsDirty */);
    FOnSkillDataDirty OnSkillDataDirty;

    /** Used fot SkillTable only */
    DECLARE_DELEGATE_OneParam(FOnSkillIdChanged, FActionSkillScope&/*ActionSkill Point*/);
    FOnSkillIdChanged OnSkillIdChanged;

	DECLARE_DELEGATE(FOnSelectedEventIdChanged);
	FOnSelectedEventIdChanged OnSelectedEventIdChanged;
private:
    bool bDirty;
#endif
};

typedef TSharedPtr<struct FActionSkillScope> FActionSkillPtr;
#if false
typedef TIndexedContainerIterator<const TArray<FActionSkillPtr>, const FActionSkillPtr, int32> SkillConstIterator;
#else
typedef TMap<int32, FActionSkillPtr>::TIterator SkillIterator;
typedef TMap<int32, FActionSkillPtr>::TConstIterator SkillConstIterator;
#endif

// DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnActionKeyEventNotified, FActionSkillPtr, InSkill, const FAction&, InAction, const FActionKeyFrame&, KeyFrame);
DECLARE_DELEGATE_FourParams(FOnActionKeyEventNotified, UObject* /*SkillCaster*/, FActionSkillScope*,/* InSkill,*/ const FAction&,/* InAction,*/ const FActionKeyFrame&/* , KeyFrame*/);
DECLARE_DELEGATE_TwoParams(FIndexEffectVariable, void* /*lua_State*/, const FUserStructOnScope& /*Data*/);

/*********************************************************************/
// UActionSkillTable
/*********************************************************************/
UCLASS()
class ACTIONSKILL_API UActionSkillTable : public UObject
{
    GENERATED_BODY()

    friend class FActionSkillTableCSVExporter;
    friend class FActionSkillTableCSVImporter;
    friend class FActionSkillModule;

public:
    UActionSkillTable(const FObjectInitializer& ObjectInitializer);

    /** Empty skill table */
    void EmptyTable();

    /** Get base skill ustruct */
    static UScriptStruct& GetSkillStruct();

    /** Get action frame sub-event struct type */
    static UScriptStruct& GetSubEventStruct();

    /** create iterator for traverse */
    SkillConstIterator CreateConstIterator() const { return SkillConstIterator(ActionSkills); }
    
    /** create iterator for traverse */
    SkillIterator CreateIterator() { return SkillIterator(ActionSkills); }

private:
    /** Inner data container */
    // TArray<FActionSkillPtr> ActionSkills;
    TMap<int32, FActionSkillPtr> ActionSkills;

#if WITH_EDITOR
public:
    /** Create new actionskill and push to skill table */
    FActionSkillPtr NewActionSkill();
    /** Delete a actionskill */
    void DeleteActionSkill(FActionSkillPtr InActionSkill);
	/** Delete a actionevent */
	void DeleteActionEvent(int32 Id);
    /** Callback when save table */
    virtual void OnSaveData();
private:
    /** We need remap the skilltable when skill id change */
    void OnSkillIdChanged(FActionSkillScope& InSkill);
	void OnSelectedEventIdChanged();

    static void UpdateNextValideSkillId(int32 NewId);
#endif

};

/*********************************************************************/
// FActionSkillModule
/*********************************************************************/
class ACTIONSKILL_API FActionSkillModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;

    void RegisterActionKeyNotifyHandler(const UStruct* Struct, FOnActionKeyEventNotified Handler);
	void RegisterEffectVariableHandler(const UStruct* Struct, FIndexEffectVariable Handler);
    FOnActionKeyEventNotified* SearchActionNotifyHandler(const UStruct* Struct);
	FIndexEffectVariable* SearchEffectVariableHandler(const UStruct* Struct);

    FORCEINLINE UActionSkillTable* GetSkillTable() { return SkillTable; }
    FORCEINLINE UActionEventTable* GetEventTable() { return EventTable; }
    FActionSkillPtr GetSkillById(int32 Id);
    FActionEventPtr GetActionEventById(int32 Id);

    bool LoadSkillConfig();

#if WITH_EDITOR
    bool ExportSkillConfigs(FString& OutSkills, FString& OutEvents);
    void PostOnSaveData();
#endif
	static FActionSkillModule& Get();
private:
    TMap<const UStruct* , FOnActionKeyEventNotified> RegisteredActionKeyNotifyHandler;
	TMap<const UStruct* , FIndexEffectVariable> RegisteredEffectVariableHandler;

    UActionSkillTable*                      SkillTable;
    UActionEventTable*                      EventTable;
};