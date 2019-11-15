// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-23

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"
#include "UObject/PropertyPortFlags.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/UserDefinedStruct.h"
#include "DataTableUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAtelierSerialization, Log, All)

struct FLuaValue;
struct FLuaTable;

struct COMMONLIBS_API FSerializeResult
{
public:
    FSerializeResult() = default;

    FORCEINLINE bool HasError() { return OutProblems.Num() > 0; }

    void AddProblem(const FString& InProblem)
    {
        OutProblems.Add(InProblem);
    }

    FString ToString() 
    {
        FString Result;
        if (HasError())
        {
            for (int i = 0; i < OutProblems.Num(); ++ i)
            {
                Result += OutProblems[i] + "\n";
            }
        }
        else
        {
            Result = "No Error.";
        }
        return Result;
    }
public:
    TArray<FString> OutProblems;
};

typedef TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>> FSerializeJsonWriter;

#pragma region SerializeUtils
namespace SerializeUtils
{
    enum EFlag
    {
        // 使用String进行序列化
        UsePlainString,
        // 使用Json进行序列化
        UseJson,
        // 使用Lua进行序列化
        UseLuaString,
        SerializeMethodMask = 0xff
    };

    FORCEINLINE EFlag GetSerializeMethod(EFlag Method) { return EFlag(SerializeMethodMask & Method); }

    FORCEINLINE EDataTableExportFlags GetCommonExportFlags()
    {
        return EDataTableExportFlags::UsePrettyPropertyNames | EDataTableExportFlags::UsePrettyEnumNames | EDataTableExportFlags::UseJsonObjectsForStructs;
    }

    FString COMMONLIBS_API AssignStringToProperty(const FString& InString, const UProperty* InProp, uint8* InData, EFlag Flags = UsePlainString);
    FString COMMONLIBS_API AssignStringToPropertyDirect(const FString& InString, const UProperty* InProp, uint8* InData);

#if WITH_EDITOR
    FString COMMONLIBS_API GetPropertyValueAsString(const UProperty* InProp, const uint8* InData, EFlag Flags = UsePlainString);
    FString COMMONLIBS_API GetPropertyValueAsStringDirect(const UProperty* InProp, const uint8* InData);

    FORCEINLINE FString GetPropertyExportName(const UProperty* InProp)
    {
        return DataTableUtils::GetPropertyExportName(InProp, GetCommonExportFlags());
    }
#endif
}
#pragma endregion SerializeUtils

namespace StructSerializer
{
    bool COMMONLIBS_API ImportJson(const TSharedRef<class FJsonObject>& InJsonObject, UScriptStruct* InStructType, void* InStructMemory, FSerializeResult& OutResult);
    bool COMMONLIBS_API ImportString(const FString& InString, UScriptStruct* InStructType, void* InStructMemory, const int32 InPortFlags, const FString& StructName, FSerializeResult& OutResult);
    bool COMMONLIBS_API ImportCSVRow( const TMap<FName, int32>& Headers, const TArray<const TCHAR*>& RowCells,
                                    const UScriptStruct* InStructType, void* InStructMemory,
                                    SerializeUtils::EFlag InnerPropFlags, FSerializeResult& OutResult);
    bool COMMONLIBS_API ImportJsonString(const FString& InString, UScriptStruct* InStructType, void* InStructMemory, FSerializeResult& OutResult);
    bool COMMONLIBS_API ImportLuaString(const FString& InString, const UScriptStruct* InStructType, void* InStructMemory, FSerializeResult& OutResult);
    bool COMMONLIBS_API ImportLuaString(const TSharedPtr<FLuaTable>& InLuaObject, const UScriptStruct* InStructType, void* InStructMemory, FSerializeResult& OutResult);

    bool COMMONLIBS_API ExportString(FString& OutString, const UScriptStruct* InStructType, const void* InStructMemory, const int32 InPortFlags);
#if WITH_EDITOR
    bool COMMONLIBS_API ExportCSVRow( FString& OutString, const UScriptStruct* InStructType, void* InStructMemory, SerializeUtils::EFlag InnerPropFlags, FSerializeResult& OutResult);
    bool COMMONLIBS_API ExportJson(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const UScriptStruct* InStructType, const void* InStructMemory);
    bool COMMONLIBS_API ExportJsonString(FString& OutString, const UScriptStruct* InStructType, const void* InStructMemory);
    bool COMMONLIBS_API ExportJsonStringWithTrim(FString& OutString, const UScriptStruct* InStructType, const void* InStructMemory);
    bool COMMONLIBS_API ExportLuaString(FString& OutString, const UScriptStruct* InStructType, const void* InStructMemory);
#endif
}

namespace PropertySerializer
{
#pragma region Serializer with json
    /** For Element in Array | Set | Map */
    bool ImportContainerEntryJson(const TSharedRef<class FJsonValue>& InJsonValue, int32 EntryIndex, const FString& ContainerPropName, UProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult);

    bool ImportJson(const TSharedRef<class FJsonValue>& InJsonValue, const UProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult);
    /** Common property */
    bool ImportJson_Base(const TSharedRef<class FJsonValue>& InJsonValue, const UProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult);
    /** For Enum */
    bool ImportJson_Enum(const TSharedRef<class FJsonValue>& InJsonValue, const UEnumProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult);
    /** For Number */
    bool ImportJson_Numeric(const TSharedRef<class FJsonValue>& InJsonValue, const UNumericProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult);
    /** For Boolean */
    bool ImportJson_Bool(const TSharedRef<class FJsonValue>& InJsonValue, const UBoolProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult);
    /** For Array */
    bool ImportJson_Array(const TSharedRef<class FJsonValue>& InJsonValue, const UArrayProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult);
    /** For Set */
    bool ImportJson_Set(const TSharedRef<class FJsonValue>& InJsonValue, const USetProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult);
    /** For Map */
    bool ImportJson_Map(const TSharedRef<class FJsonValue>& InJsonValue, const UMapProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult);
    /** For Struct */
    bool ImportJson_Struct(const TSharedRef<class FJsonValue>& InJsonValue, const UStructProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult);

#if WITH_EDITOR
    bool ExportJson(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const UProperty* InProperty, void* InPropertyData);

    bool ExportJson(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UProperty* InProperty, void* InPropertyData);
    /** For Enum */
    bool ExportJson_Enum(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UEnumProperty* InProperty, void* InPropertyData);
    /** For Number */
    bool ExportJson_Numeric(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UNumericProperty* InProperty, void* InPropertyData);
    /** For Boolean */
    bool ExportJson_Bool(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UBoolProperty* InProperty, void* InPropertyData);
    /** For Array */
    bool ExportJson_Array(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UArrayProperty* InProperty, void* InPropertyData);
    /** For Set */
    bool ExportJson_Set(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const USetProperty* InProperty, void* InPropertyData);
    /** For Map */
    bool ExportJson_Map(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UMapProperty* InProperty, void* InPropertyData);
    /** For Struct */
    bool ExportJson_Struct(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UStructProperty* InProperty, void* InPropertyData);
#endif
#pragma endregion

#pragma region Serializer with text
    bool ImportString(const FString& InString, const UProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult);
    bool ImportString_Base(const FString& InString, const UProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult);
    bool ImportString_Array(const FString& InString, const UArrayProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult);
    bool ImportString_Set(const FString& InString, const USetProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult);
    bool ImportString_Map(const FString& InString, const UMapProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult);
    bool ImportString_Struct(const FString& InString, const UStructProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult);

#if WITH_EDITOR
    bool ExportString(FString& OutString, const UProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags);
    bool ExportString_Base(FString& OutString, const UProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags);
    bool ExportString_Array(FString& OutString, const UArrayProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags);
    bool ExportString_Set(FString& OutString, const USetProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags);
    bool ExportString_Map(FString& OutString, const UMapProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags);
    bool ExportString_Struct(FString& OutString, const UStructProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags);
#endif
#pragma endregion

#pragma region Serializer with lua
    bool ImportContainerEntryLua(const TSharedRef<FLuaValue>& InLuaValue, int32 EntryIndex, const FString& ContainerPropName, UProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult);
    bool ImportLua(const TSharedRef<FLuaValue>& InLuaValue, const UProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult);
    bool ImportLua_Base(const TSharedRef<FLuaValue>& InLuaValue, const UProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult);
    bool ImportLua_Enum(const TSharedRef<FLuaValue>& InLuaValue, const UEnumProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult);
    bool ImportLua_Numeric(const TSharedRef<FLuaValue>& InLuaValue, const UNumericProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult);
    bool ImportLua_Bool(const TSharedRef<FLuaValue>& InLuaValue, const UBoolProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult);
    bool ImportLua_Array(const TSharedRef<FLuaValue>& InLuaValue, const UArrayProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult);
    bool ImportLua_Set(const TSharedRef<FLuaValue>& InLuaValue, const USetProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult);
    bool ImportLua_Map(const TSharedRef<FLuaValue>& InLuaValue, const UMapProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult);
    bool ImportLua_Struct(const TSharedRef<FLuaValue>& InLuaValue, const UStructProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult);

#if WITH_EDITOR
    bool ExportLua(FString& OutString, const UProperty* InProperty, const uint8* InPropertyData);
    bool ExportLua_Base(FString& OutString, const UProperty* InProperty, const uint8* InPropertyData);
    bool ExportLua_Enum(FString& OutString, const UEnumProperty* InProperty, const uint8* InPropertyData);
    bool ExportLua_Numeric(FString& OutString, const UNumericProperty* InProperty, const uint8* InPropertyData);
    bool ExportLua_Bool(FString& OutString, const UBoolProperty* InProperty, const uint8* InPropertyData);
    bool ExportLua_Array(FString& OutString, const UArrayProperty* InProperty, const uint8* InPropertyData);
    bool ExportLua_Set(FString& OutString, const USetProperty* InProperty, const uint8* InPropertyData);
    bool ExportLua_Map(FString& OutString, const UMapProperty* InProperty, const uint8* InPropertyData);
    bool ExportLua_Struct(FString& OutString, const UStructProperty* InProperty, const uint8* InPropertyData);
#endif
#pragma endregion
}