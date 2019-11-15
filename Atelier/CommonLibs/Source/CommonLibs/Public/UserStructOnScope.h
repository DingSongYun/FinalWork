// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-23

#pragma once

#include "CoreMinimal.h"
#include "UObject/StructOnScope.h"

#include "UserStructOnScope.generated.h"

/*********************************************************************/
// FUserStructOnScope
// 本类用来封装一些不定类型的Struct, 和提供一致性的序列化成string的方法
// 并且保存到一个UPROPERTY字段来方便其他UE4本身的保存体系
/*********************************************************************/
USTRUCT(BlueprintType, Blueprintable)
struct COMMONLIBS_API FUserStructOnScope
{
    GENERATED_BODY()

public:
    /**
     * Constructor
     * @param UserStruct            [UScriptStruct] user struct type info
     */
    FUserStructOnScope();

    virtual ~FUserStructOnScope() {}
    void Initialize(const UScriptStruct& UserStruct);
    void Reset();

    //~ Begin: Struct Functions
    // bool Serialize(FArchive& Ar);
    bool ExportTextItem(FString& ValueStr, FUserStructOnScope const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const;
    bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText);
    //~ End: Struct Functions

    /**
     * Get generic un-typed struct
     */
    // FORCEINLINE TSharedPtr<FStructOnScope> GetData() { return InnerStruct; }
    FORCEINLINE TSharedPtr<FStructOnScope> GetData() const { return InnerStruct; }

    /**
     * Get Property by name
     */
    UProperty* GetStructPropertyByName(const FName& InName);

    /**
     * Set Property value by name
     */
    void SetStructPropertyValue(const FName& PropName, int32 InValue);

    uint8* GetStructMemory()
    {
        return GetData()->GetStructMemory();
    }

    const uint8* GetStructMemory() const
    {
        return GetData()->GetStructMemory();
    }

    const UStruct* GetStruct() const
    {
        return GetData()->GetStruct();
    }

    virtual bool IsValid() const
    {
        return InnerStruct.IsValid() && InnerStruct->IsValid();
    }

    /**
     * Parse InnerStruct from SerializeOnString and 
     */
    void InnerImport();

#if WITH_EDITOR
    void InnerExport();
    bool CanChangeUserStructType() { return !IsValid() || !bFixedStructType; }
    void FixedUserStructType() { bFixedStructType = true; }

    void SetUserStructBaseType(UClass* BaseStruct) { UserStructBaseType = BaseStruct; }
    UClass* GetUserStructBaseType() { return UserStructBaseType; }
    bool bFixedStructType;
    UClass* UserStructBaseType;
#endif

private:
    /** Serialized string of InnerStruct */
    UPROPERTY()
    FString SerializeOnString;

    /** Struct path of user struct type */
    UPROPERTY()
    FSoftObjectPath		UserStructPath;

    /** Struct store data */
    TSharedPtr<FStructOnScope> InnerStruct;

    /** Cached prop map for fast find prop by name */
    TMap<FName, UProperty*> PropDict;
};

// FArchive& operator<<(class FArchive& Ar, TSharedPtr<FStructOnScope>& UserStructOnScope);

template<>
struct TStructOpsTypeTraits<FUserStructOnScope> : public TStructOpsTypeTraitsBase2<FUserStructOnScope>
{
    enum
    {
        WithExportTextItem = true,
        WithImportTextItem = true,
        // WithSerializer = true,
    };
};