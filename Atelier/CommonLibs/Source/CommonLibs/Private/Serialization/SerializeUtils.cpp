#include "Serialization/SerializeUtils.h"
#include "Engine/UserDefinedEnum.h"
#include "UserStructOnScope.h"
#include "LuaSerializeHelper.h"

DEFINE_LOG_CATEGORY(LogAtelierSerialization);

// TODO: Serialization of json is uncomplete, do it later
#define PROP_PORT_STRING_WITH_JSON 0

const TCHAR* JSONTypeToString(const EJson InType)
{
    switch(InType)
    {
    case EJson::None:
        return TEXT("None");
    case EJson::Null:
        return TEXT("Null");
    case EJson::String:
        return TEXT("String");
    case EJson::Number:
        return TEXT("Number");
    case EJson::Boolean:
        return TEXT("Boolean");
    case EJson::Array:
        return TEXT("Array");
    case EJson::Object:
        return TEXT("Object");
    default:
        return TEXT("Unknown");
    }
}

#pragma region SerializeUtils
#if WITH_EDITOR
FString SerializeUtils::GetPropertyValueAsString(const UProperty* InProp, const uint8* InData, EFlag Flags)
{
    FString OutString;
    switch(GetSerializeMethod(Flags))
    {
	case SerializeUtils::UseLuaString:
        PropertySerializer::ExportLua(OutString, InProp, InData);
        break;
    default:
        PropertySerializer::ExportString(OutString, InProp, InData, PPF_None);
    }
    return OutString;
    // return DataTableUtils::GetPropertyValueAsString(InProp, InData, GetCommonExportFlags());
}

FString SerializeUtils::GetPropertyValueAsStringDirect(const UProperty* InProp, const uint8* InData)
{
    FString OutString;
    PropertySerializer::ExportString(OutString, InProp, InData, PPF_None);
    return OutString;
    // return DataTableUtils::GetPropertyValueAsStringDirect(InProp, InData, GetCommonExportFlags());
}
#endif

FString SerializeUtils::AssignStringToProperty(const FString& InString, const UProperty* InProp, uint8* InData, EFlag Flags)
{
    FSerializeResult OutResult;
    EFlag Method = GetSerializeMethod(Flags);
    if (Method == SerializeUtils::UsePlainString)
    {
        PropertySerializer::ImportString(InString, InProp, InData, PPF_None, OutResult);
    }
    else if (Method == SerializeUtils::UseLuaString)
    {
        FLuaReader LuaReader(InString);
        if (LuaReader.LuaValue.IsValid())
        {
            PropertySerializer::ImportLua(LuaReader.LuaValue.ToSharedRef(), InProp, InData, OutResult);
        }
    }
    return OutResult.ToString();
    // return DataTableUtils::AssignStringToProperty(InString, InProp, InData);
}

FString SerializeUtils::AssignStringToPropertyDirect(const FString& InString, const UProperty* InProp, uint8* InData)
{
    FSerializeResult OutResult;
    PropertySerializer::ImportString(InString, InProp, InData, PPF_None, OutResult);
    return OutResult.ToString();
    // return DataTableUtils::AssignStringToPropertyDirect(InString, InProp, InData);
} 
#pragma endregion

#pragma region StructSerializer
bool StructSerializer::ImportJson(const TSharedRef<class FJsonObject>& InJsonObject, UScriptStruct* InStructType, void* InStructMemory, FSerializeResult& OutResult)
{
    check(InStructType);
    check(InStructMemory);

    for (TFieldIterator<UProperty> It(InStructType); It; ++It)
    {
        UProperty* BaseProperty = *It;
        check(BaseProperty);

        const FString PropName = DataTableUtils::GetPropertyDisplayName(BaseProperty, BaseProperty->GetName());

        TSharedPtr<FJsonValue> JsonValueToParse;
        // prop name可能存在不确定性
        for (const FString& ImpName : DataTableUtils::GetPropertyImportNames(BaseProperty))
        {
            JsonValueToParse = InJsonObject->TryGetField(ImpName);
            if (JsonValueToParse.IsValid()) break;
        }

        // 没找到对应的可序列化字段
        if (!JsonValueToParse.IsValid())
        {
            OutResult.AddProblem(FString::Printf(TEXT("Property %s missing an value"), *PropName));
            continue;
        }

        // 处理非Array
        if (BaseProperty->ArrayDim == 1)
        {
            void* Data = BaseProperty->ContainerPtrToValuePtr<void>(InStructMemory, 0);
            return PropertySerializer::ImportJson(JsonValueToParse.ToSharedRef(), BaseProperty, Data, OutResult);
        }
        // 处理Array
        else
        {
            const TArray< TSharedPtr<FJsonValue> >* PropertyValuesArray;
            if (!JsonValueToParse->TryGetArray(PropertyValuesArray))
            {
                OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected Array, got %s."), *PropName, JSONTypeToString(JsonValueToParse->Type)));
                return false;
            }

            if (BaseProperty->ArrayDim != PropertyValuesArray->Num())
            {
                OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is a static sized array with %d elements, but we have %d values to import"), *PropName, BaseProperty->ArrayDim, PropertyValuesArray->Num()));
            }

            for (int32 Index = 0; Index < BaseProperty->ArrayDim; ++Index)
            {
                if (PropertyValuesArray->IsValidIndex(Index))
                {
                    void* Data = BaseProperty->ContainerPtrToValuePtr<void>(InStructMemory, Index);
                    const TSharedPtr<FJsonValue>& PropValue = (*PropertyValuesArray)[Index];
                    PropertySerializer::ImportContainerEntryJson(PropValue.ToSharedRef(), Index, BaseProperty->GetName(), BaseProperty, Data, OutResult);
                }
            }
        }
    }
    return true;
}

bool StructSerializer::ImportString(const FString& InString, UScriptStruct* InStructType, void* InStructMemory, const int32 InPortFlags, const FString& StructName, FSerializeResult& OutResult)
{
    FStringOutputDevice ImportError;
    InStructType->ImportText(*InString, InStructMemory, nullptr, InPortFlags, &ImportError, StructName);
    if (!ImportError.IsEmpty())
    {
        OutResult.AddProblem(ImportError);
    }
    return true;
}

bool StructSerializer::ImportCSVRow( const TMap<FName, int32>& Headers, const TArray<const TCHAR*>& RowCells,
                                const UScriptStruct* InStructType, void* InStructMemory,
                                SerializeUtils::EFlag InnerPropFlags, FSerializeResult& OutResult)
{
    for (TFieldIterator<UProperty> PropIt(InStructType); PropIt; ++PropIt)
    {
        UProperty* Property = *PropIt;
        //FName PropertyName = FName(*Property->GetShowName());
        FName PropertyName = FName(*SerializeUtils::GetPropertyExportName(Property));

        if (!Headers.Contains(PropertyName))
        {
            OutResult.AddProblem(FString::Printf(TEXT("Skill has more properies(%s) than csv row"), *PropertyName.ToString()));
            continue;
        }

        int32 ColumnIndex = Headers.FindRef(PropertyName);
        FString PropertyString = RowCells[ColumnIndex];
        PropertyString = PropertyString.Replace(TEXT("'"), TEXT("\""));
        uint8* Data = (uint8*)Property->ContainerPtrToValuePtr<void>(InStructMemory, 0);
        FString Error = SerializeUtils::AssignStringToProperty(PropertyString, Property, Data, InnerPropFlags);
        if(Error.Len() > 0)
        {
            FString ColumnName = (Property != NULL) 
                ? DataTableUtils::GetPropertyDisplayName(Property, Property->GetName())
                : FString(TEXT("NONE"));
            OutResult.AddProblem(FString::Printf(TEXT("Problem assigning string '%s' to property '%s' : %s"), RowCells[ColumnIndex], *ColumnName, *Error));
        }
    }

    return true;
}


bool StructSerializer::ImportJsonString(const FString& InString, UScriptStruct* InStructType, void* InStructMemory, FSerializeResult& OutResult)
{
    const TSharedRef< TJsonReader<TCHAR> > JsonReader = TJsonReaderFactory<TCHAR>::Create(InString);
    TSharedPtr<FJsonObject> JsonObject;
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
    {
        OutResult.AddProblem(FString::Printf(TEXT("JsonObjectStringToUStruct - Unable to parse json=[%s]"), *InString));
        return false;
    }
    StructSerializer::ImportJson(JsonObject.ToSharedRef(), InStructType, InStructMemory, OutResult);
    return true;
}

bool StructSerializer::ImportLuaString(const FString& InString, const UScriptStruct* InStructType, void* InStructMemory, FSerializeResult& OutResult)
{
    check(InStructType);
    check(InStructMemory);

    FLuaReader LuaReader(InString);
    return ImportLuaString(StaticCastSharedPtr<FLuaTable>(LuaReader.LuaValue), InStructType, InStructMemory, OutResult);
}

bool StructSerializer::ImportLuaString(const TSharedPtr<FLuaTable>& InLuaObject, const UScriptStruct* InStructType, void* InStructMemory, FSerializeResult& OutResult)
{
    check(InStructType);
    check(InStructMemory);
    check(InLuaObject.IsValid());

    if (InStructType == FUserStructOnScope::StaticStruct())
    {
        if (FUserStructOnScope* StructPtr = (FUserStructOnScope*)(InStructMemory))
        {
            TSharedPtr<FLuaProp> StructTypeProp = InLuaObject->GetPropByName(LUA_USER_STRUCT_TYPE_PROP);
            if (StructTypeProp.IsValid())
            {
                FSoftObjectPath StructTypePath(StructTypeProp->Value->GetString());

                if(UScriptStruct* UserStruct = Cast<UScriptStruct>(StructTypePath.TryLoad()))
                {
                    StructPtr->Initialize(*UserStruct);
                }
                if (StructPtr->IsValid())
                {
                    void* InnerStructData = StructPtr->GetStructMemory();
                    UScriptStruct* InnerStructType = const_cast<UScriptStruct*>(Cast<const UScriptStruct>(StructPtr->GetStruct()));
                    return StructSerializer::ImportLuaString(InLuaObject, InnerStructType, InnerStructData, OutResult);
                }
            }
        }
    }

    for (TFieldIterator<UProperty> It(InStructType); It; ++It)
    {
        UProperty* BaseProperty = *It;
        check(BaseProperty);

        const FString PropName = DataTableUtils::GetPropertyDisplayName(BaseProperty, BaseProperty->GetName());

        TSharedPtr<FLuaProp> LuaPropToParse;
        // prop name可能存在不确定性
        for (const FString& ImpName : DataTableUtils::GetPropertyImportNames(BaseProperty))
        {
            LuaPropToParse = InLuaObject->GetPropByName(ImpName);
            if (LuaPropToParse.IsValid()) break;
        }

        TSharedPtr<FLuaValue> LuaValueToParse = LuaPropToParse.IsValid() ? LuaPropToParse->Value : nullptr;
        // 没找到对应的可序列化字段
        if (!LuaValueToParse.IsValid())
        {
            OutResult.AddProblem(FString::Printf(TEXT("Property %s missing an value"), *PropName));
            continue;
        }

        void* Data = BaseProperty->ContainerPtrToValuePtr<void>(InStructMemory, 0);
        PropertySerializer::ImportLua(LuaValueToParse.ToSharedRef(), BaseProperty, (uint8*)Data, OutResult);

        #if 0
        // 处理非Array
        if (BaseProperty->ArrayDim == 1)
        {
            void* Data = BaseProperty->ContainerPtrToValuePtr<void>(InStructMemory, 0);
            return PropertySerializer::ImportLua(LuaValueToParse.ToSharedRef(), BaseProperty, (uint8*)Data, OutResult);
        }
        // 处理Array
        else
        {
            TArray< TSharedPtr<FLuaValue> > PropertyValuesArray;
            if (!LuaValueToParse->TryGetArray(PropertyValuesArray))
            {
                OutResult.AddProblem( FString::Printf(TEXT("Property '%s' is the incorrect type. Expected Array, got %s."), *PropName, *LuaValueToParse->GetString()) );
                return false;
            }

            if (BaseProperty->ArrayDim != PropertyValuesArray.Num())
            {
                OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is a static sized array with %d elements, but we have %d values to import"), *PropName, BaseProperty->ArrayDim, PropertyValuesArray.Num()));
            }

            for (int32 Index = 0; Index < BaseProperty->ArrayDim; ++Index)
            {
                if (PropertyValuesArray.IsValidIndex(Index))
                {
                    void* Data = BaseProperty->ContainerPtrToValuePtr<void>(InStructMemory, Index);
                    const TSharedPtr<FLuaValue>& PropValue = PropertyValuesArray[Index];
                    PropertySerializer::ImportContainerEntryLua(PropValue.ToSharedRef(), Index, BaseProperty->GetName(), BaseProperty, (uint8*)Data, OutResult);
                }
            }
        }
        #endif
    }
    return true;
}

bool StructSerializer::ExportString(FString& OutString, const UScriptStruct* InStructType, const void* InStructMemory, const int32 InPortFlags)
{
    InStructType->ExportText(OutString, InStructMemory, InStructMemory, nullptr, InPortFlags, nullptr);

    return true;
}

#if WITH_EDITOR
bool StructSerializer::ExportCSVRow(FString& OutString, const UScriptStruct* InStructType, void* InStructMemory, SerializeUtils::EFlag InnerPropFlags, FSerializeResult& OutResult)
{
    for (TFieldIterator<UProperty> It(InStructType); It; ++It)
    {
        UProperty* BaseProp = *It;
        check(BaseProp);

        const uint8* Data = (uint8*)BaseProp->ContainerPtrToValuePtr<void>(InStructMemory, 0);
        const FString PropertyValue = SerializeUtils::GetPropertyValueAsString(BaseProp, Data, InnerPropFlags);
        OutString += TEXT("\"");
        // ExportedText += PropertyValue.Replace(TEXT("\""), TEXT("\"\""));
        OutString += PropertyValue.Replace(TEXT("\""), TEXT("'"));
        OutString += TEXT("\"");
        OutString += TEXT(",");
    }

    return true;
}

bool StructSerializer::ExportJson(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const UScriptStruct* InStructType, const void* InStructMemory)
{
    check(InStructType);
    check(InStructMemory);
    for (TFieldIterator<const UProperty> It(InStructType); It; ++It)
    {
        const UProperty* BaseProperty = *It;
        check(BaseProperty);

        if (BaseProperty->ArrayDim == 1)
        {
            const void* Data = BaseProperty->ContainerPtrToValuePtr<void>(InStructMemory, 0);
            PropertySerializer::ExportJson(OutJsonWriter, BaseProperty, (void*)Data);
        }
        else
        {
            const FString Identifier = SerializeUtils::GetPropertyExportName(BaseProperty);
            OutJsonWriter->WriteArrayStart(Identifier);
            for (int32 Index = 0; Index < BaseProperty->ArrayDim; ++Index)
            {
                const void* Data = BaseProperty->ContainerPtrToValuePtr<void>(InStructType, Index);
                PropertySerializer::ExportJson(OutJsonWriter, nullptr, BaseProperty, (void*)Data);
            }
            OutJsonWriter->WriteArrayEnd();
        }
    }

    return true;
}

bool StructSerializer::ExportJsonString(FString& OutString, const UScriptStruct* InStructType, const void* InStructMemory)
{
    TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutString);
    JsonWriter->WriteObjectStart();
    StructSerializer::ExportJson(JsonWriter, InStructType, InStructMemory);
    JsonWriter->WriteObjectEnd();
    JsonWriter->Close();
    return true;
}

bool StructSerializer::ExportJsonStringWithTrim(FString& OutString, const UScriptStruct* InStructType, const void* InStructMemory)
{
    StructSerializer::ExportJsonString(OutString, InStructType, InStructMemory);

    OutString.ReplaceInline(TEXT("\t"), TEXT(""), ESearchCase::CaseSensitive);
    OutString.ReplaceInline(TEXT("\r"), TEXT(""), ESearchCase::CaseSensitive);
    OutString.ReplaceInline(TEXT("\n"), TEXT(" "), ESearchCase::CaseSensitive);

    return true;
}

bool StructSerializer::ExportLuaString(FString& OutString, const UScriptStruct* InStructType, const void* InStructMemory)
{
    check(InStructType);
    check(InStructMemory);
    
    if (InStructType == FUserStructOnScope::StaticStruct())
    {
        if (const FUserStructOnScope* StructPtr = (const FUserStructOnScope*)(InStructMemory))
        {
            if (StructPtr->IsValid())
            {
                const void* StructData = StructPtr->GetStructMemory();
                const UScriptStruct* StructType = Cast<const UScriptStruct>(StructPtr->GetStruct());

                FString InnerStructString;
                if (StructSerializer::ExportLuaString(InnerStructString, StructType, StructData))
                {
                    if (const UScriptStruct* UserStruct = Cast<UScriptStruct>(StructPtr->GetStruct()))
                    {
                        FSoftObjectPath ObjectPath(UserStruct);
                        OutString += LUA_USER_STRUCT_TYPE_PROP + "='" + ObjectPath.GetAssetPathString() + "'" + LUA_PROP_SPLIT;    // 不定Struct的类型
                        OutString += InnerStructString;
                    }
                }

            }
        }
    }
    else
    {
        for (TFieldIterator<const UProperty> It(InStructType); It; ++It)
        {
            const UProperty* BaseProperty = *It;
            check(BaseProperty);

            // Mark new property
            FLuaSerializeHelper::WriteLuaPropStartString(OutString, BaseProperty);

            if (BaseProperty->ArrayDim == 1)
            {
                const uint8* Data = BaseProperty->ContainerPtrToValuePtr<uint8>(InStructMemory, 0);
                PropertySerializer::ExportLua(OutString, BaseProperty, Data);
            }
            else
            {
                FLuaSerializeHelper::WriteLuaTableStartString(OutString);
                for (int32 Index = 0; Index < BaseProperty->ArrayDim; ++Index)
                {
                    const uint8* Data = BaseProperty->ContainerPtrToValuePtr<uint8>(InStructType, Index);
                    PropertySerializer::ExportLua(OutString, BaseProperty, Data);
                }
                FLuaSerializeHelper::WriteLuaTableEndString(OutString);
            }
        }
    }
    return true;
}

#endif
#pragma endregion

#pragma region PropertySerializer
#pragma region Import
#define TYPED_IMPORT_STRING_AND_RETURN(Type) \
    if (const U##Type##Property* TypeProp = Cast<const U##Type##Property>(InProperty)) \
        return ImportString_##Type(InString, TypeProp, InPropertyData, InPortFlags, OutResult)

#define TYPED_IMPORT_JSON_AND_RETURN(Type) \
    if (const U##Type##Property* TypeProp = Cast<const U##Type##Property>(InProperty)) \
        return ImportJson_##Type(InJsonValue, TypeProp, InPropertyData, OutResult)

#define TYPED_IMPORT_LUA_AND_RETURN(Type) \
    if (const U##Type##Property* TypeProp = Cast<const U##Type##Property>(InProperty)) \
        return ImportLua_##Type(InLuaValue, TypeProp, InPropertyData, OutResult)

bool PropertySerializer::ImportJson(const TSharedRef<class FJsonValue>& InJsonValue, const UProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult)
{
    TYPED_IMPORT_JSON_AND_RETURN(Enum);
    TYPED_IMPORT_JSON_AND_RETURN(Numeric);
    TYPED_IMPORT_JSON_AND_RETURN(Bool);
    TYPED_IMPORT_JSON_AND_RETURN(Array);
    TYPED_IMPORT_JSON_AND_RETURN(Set);
    TYPED_IMPORT_JSON_AND_RETURN(Map);
    TYPED_IMPORT_JSON_AND_RETURN(Struct);

    return ImportJson_Base(InJsonValue, InProperty, InPropertyData, OutResult);
}

/** Common property */
bool PropertySerializer::ImportJson_Base(const TSharedRef<class FJsonValue>& InJsonValue, const UProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult)
{
    FString PropertyValue;
    if (!InJsonValue->TryGetString(PropertyValue))
    {
        OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected String, got %s."), *InProperty->GetName(), JSONTypeToString(InJsonValue->Type)));
        return false;
    }

    FString Error = SerializeUtils::AssignStringToProperty(PropertyValue, InProperty, (uint8*)InPropertyData);
    if (!Error.IsEmpty())
    {
        OutResult.AddProblem(FString::Printf(TEXT("Property assigning string '%s' to property '%s' : %s "), *PropertyValue, *InProperty->GetName(), *Error));
        return false;
    }

    return true;
}

/** For Enum */
bool PropertySerializer::ImportJson_Enum(const TSharedRef<class FJsonValue>& InJsonValue, const UEnumProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult)
{
    FString EnumStrValue;
    if (InJsonValue->TryGetString(EnumStrValue))
    {
        FString Error = SerializeUtils::AssignStringToProperty(EnumStrValue, InProperty, (uint8*)InPropertyData);
        if (!Error.IsEmpty())
        {
            OutResult.AddProblem(FString::Printf(TEXT("Property '%s' has invalid enum value: %s."), *InProperty->GetName(), *EnumStrValue));
            return false;
        }
    }
    else
    {
        int64 EnumIntValue = 0;
        if (!InJsonValue->TryGetNumber(EnumIntValue))
        {
            OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected Integer, got %s."), *InProperty->GetName(),  JSONTypeToString(InJsonValue->Type)));
            return false;
        }
        InProperty->GetUnderlyingProperty()->SetIntPropertyValue(InPropertyData, EnumIntValue);
    }

    return true;
}

/** For Number */
bool PropertySerializer::ImportJson_Numeric(const TSharedRef<class FJsonValue>& InJsonValue, const UNumericProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult)
{
    FString StrValue;
    // Enum
    if (InProperty->IsEnum() && InJsonValue->TryGetString(StrValue))
    {
        FString Error = SerializeUtils::AssignStringToProperty(StrValue, InProperty, (uint8*)InPropertyData);
        if (!Error.IsEmpty())
        {
            OutResult.AddProblem(FString::Printf(TEXT("Property '%s' has invalid enum value: %s."), *InProperty->GetName(), *StrValue));
            return false;
        }
    }
    // Integer
    else if (InProperty->IsInteger())
    {
        int64 IntValue = 0;
        if (!InJsonValue->TryGetNumber(IntValue))
        {
            OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected Integer, got %s."), *InProperty->GetName(),  JSONTypeToString(InJsonValue->Type)));
            return false;
        }
        InProperty->SetIntPropertyValue(InPropertyData, IntValue);
    }
    // Double
    else
    {
        double DoubleValue = 0.0;
        if (!InJsonValue->TryGetNumber(DoubleValue))
        {
            OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected Double, got %s."), *InProperty->GetName(),  JSONTypeToString(InJsonValue->Type)));
            return false;
        }
        InProperty->SetFloatingPointPropertyValue(InPropertyData, DoubleValue);
    }

    return true;
}

/** For Boolean */
bool PropertySerializer::ImportJson_Bool(const TSharedRef<class FJsonValue>& InJsonValue, const UBoolProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult)
{
    bool PropertyValue = false;
    if (!InJsonValue->TryGetBool(PropertyValue))
    {
        OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected Boolean, got %s."), *InProperty->GetName(),  JSONTypeToString(InJsonValue->Type)));
        return false;
    }
    InProperty->SetPropertyValue(InPropertyData, PropertyValue);
    return true;
}

/** For Array */
bool PropertySerializer::ImportJson_Array(const TSharedRef<class FJsonValue>& InJsonValue, const UArrayProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult)
{
    const TArray< TSharedPtr<FJsonValue> >* ArrayValuePtr;
    if (!InJsonValue->TryGetArray(ArrayValuePtr))
    {
        OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected Array, got %s."), *InProperty->GetName(), JSONTypeToString(InJsonValue->Type)));
        return false;
    }

    FScriptArrayHelper ArrayHelper(InProperty, InPropertyData);
    ArrayHelper.EmptyValues();
    for (const TSharedPtr<FJsonValue>& ValueEntry : *ArrayValuePtr)
    {
        const int32 NewEntryIndex = ArrayHelper.AddValue();
        uint8* EntryData = ArrayHelper.GetRawPtr(NewEntryIndex);
        PropertySerializer::ImportContainerEntryJson(ValueEntry.ToSharedRef(), NewEntryIndex, InProperty->GetName(), InProperty->Inner, EntryData, OutResult);
    }
    return true;
}

/** For Set */
bool PropertySerializer::ImportJson_Set(const TSharedRef<class FJsonValue>& InJsonValue, const USetProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult)
{
    const TArray< TSharedPtr<FJsonValue> >* ArrayValuePtr;
    if (!InJsonValue->TryGetArray(ArrayValuePtr))
    {
        OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected Array, got %s."), *InProperty->GetName(), JSONTypeToString(InJsonValue->Type)));
        return false;
    }

    FScriptSetHelper SetHelper(InProperty, InPropertyData);
    SetHelper.EmptyElements();
    for (const TSharedPtr<FJsonValue>& ValueEntry : *ArrayValuePtr)
    {
        const int32 NewEntryIndex = SetHelper.AddDefaultValue_Invalid_NeedsRehash();
        uint8* SetEntryData = SetHelper.GetElementPtr(NewEntryIndex);
        PropertySerializer::ImportContainerEntryJson(ValueEntry.ToSharedRef(), NewEntryIndex, InProperty->GetName(), SetHelper.GetElementProperty(), SetEntryData, OutResult);
    }
    SetHelper.Rehash();
    return true;
}

/** For Map */
bool PropertySerializer::ImportJson_Map(const TSharedRef<class FJsonValue>& InJsonValue, const UMapProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult)
{
    const TSharedPtr<FJsonObject>* ObjectValue = nullptr;
    if (!InJsonValue->TryGetObject(ObjectValue))
    {
        OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected Object, got %s."), *InProperty->GetName(), JSONTypeToString(InJsonValue->Type)));
        return false;
    }

    FScriptMapHelper MapHelper(InProperty, InPropertyData);
    MapHelper.EmptyValues();
    for (const auto& ValuePair : (*ObjectValue)->Values)
    {
        const int32 NewEntryIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
        uint8* MapKeyData = MapHelper.GetKeyPtr(NewEntryIndex);
        uint8* MapValueData = MapHelper.GetValuePtr(NewEntryIndex);
        const FString KeyError = SerializeUtils::AssignStringToPropertyDirect(ValuePair.Key, MapHelper.GetKeyProperty(), MapKeyData);
        if (KeyError.Len() > 0)
        {
            MapHelper.RemoveAt(NewEntryIndex);
            OutResult.AddProblem(FString::Printf(TEXT("Property assigning key'%s' to property '%s' : %s "), *ValuePair.Key, *InProperty->GetName(), *KeyError));
            return false;
        }

        if (!PropertySerializer::ImportContainerEntryJson(ValuePair.Value.ToSharedRef(), NewEntryIndex, InProperty->GetName(), MapHelper.GetValueProperty(), MapValueData, OutResult))
        {
            MapHelper.RemoveAt(NewEntryIndex);
            return false;
        }
    }
    MapHelper.Rehash();
    return true;
}

/** For Struct */
bool PropertySerializer::ImportJson_Struct(const TSharedRef<class FJsonValue>& InJsonValue, const UStructProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult)
{
    const TSharedPtr<FJsonObject>* PropertyValue = nullptr;
    if (InJsonValue->TryGetObject(PropertyValue))
    {
        return StructSerializer::ImportJson(PropertyValue->ToSharedRef(), InProperty->Struct, InPropertyData, OutResult);
    }
    else
    {
        FString StringValue;
        if (!InJsonValue->TryGetString(StringValue))
        {
            OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected String, got %s."), *InProperty->GetName(), JSONTypeToString(InJsonValue->Type)));
            return false;
        }

        FString Error = SerializeUtils::AssignStringToProperty(StringValue, InProperty, (uint8*)InPropertyData);
        if (!Error.IsEmpty())
        {
            OutResult.AddProblem(FString::Printf(TEXT("Property assigning string '%s' to property '%s' : %s "), *StringValue, *InProperty->GetName(), *Error));
            return false;
        }
    }

    return true;
}

bool PropertySerializer::ImportContainerEntryJson(const TSharedRef<class FJsonValue>& InJsonValue, int32 EntryIndex, const FString& ContainerPropName, UProperty* InProperty, void* InPropertyData, FSerializeResult& OutResult)
{
    FSerializeResult OutProblem;
    bool Result = PropertySerializer::ImportJson(InJsonValue, InProperty, InPropertyData, OutProblem);
    if (Result == false || OutProblem.HasError())
    {
        OutResult.AddProblem(FString::Printf(TEXT("Entry %d on property '%s' import error begin: "), EntryIndex, *ContainerPropName));
        for (FString& Problem : OutProblem.OutProblems)
        {
            OutResult.AddProblem(Problem);
        }
        OutResult.AddProblem(FString::Printf(TEXT("Entry %d on property '%s' import error end! "), EntryIndex, *ContainerPropName));
    }
    return Result;
}

FString GetTextSourceString(const FText& Text)
{
	if (const FString* SourceString = FTextInspector::GetSourceString(Text))
	{
		return *SourceString;
	}
	return Text.ToString();
}

bool PropertySerializer::ImportString(const FString& InString, const UProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult)
{
    TYPED_IMPORT_STRING_AND_RETURN(Array);
    TYPED_IMPORT_STRING_AND_RETURN(Set);
    TYPED_IMPORT_STRING_AND_RETURN(Map);
    TYPED_IMPORT_STRING_AND_RETURN(Struct);

    return ImportString_Base(InString, InProperty, InPropertyData, InPortFlags, OutResult);
}

bool PropertySerializer::ImportString_Base(const FString& InString, const UProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult)
{
    auto DoImport = [&](const FString& InStringToImport)
    {
        FStringOutputDevice ImportError;
        InProperty->ImportText(*InStringToImport, InPropertyData, InPortFlags, nullptr, &ImportError);
        if (!ImportError.IsEmpty())
        {
            OutResult.AddProblem(ImportError);
        }
    };

    bool bNeedsImport = true;

    // SERIAL_ENUM_AS_NAME_STRING 是否用枚举名字作为序列化的值
    // 我们用枚举的值作为序列化结果，方便前后端解析
    #ifdef SERIAL_ENUM_AS_NAME_STRING
    UEnum* Enum = nullptr;

    if (const UEnumProperty* EnumProp = Cast<const UEnumProperty>(InProperty))
    {
        Enum = EnumProp->GetEnum();
    }
    else if (const UByteProperty* ByteProp = Cast<const UByteProperty>(InProperty))
    {
        if (ByteProp->IsEnum())
        {
            Enum = ByteProp->GetIntPropertyEnum();
        }
    }

    if (Enum)
    {
        const int32 EnumIndex = Enum->GetIndexByNameString(InString);
        if (EnumIndex == INDEX_NONE)
        {
            // Couldn't find a match for the name we were given, try and find a match using the friendly names
            for(int32 EnumEntryIndex = 0; EnumEntryIndex < Enum->NumEnums(); ++EnumEntryIndex)
            {
                const FText FriendlyEnumEntryName = Enum->GetDisplayNameTextByIndex(EnumEntryIndex);

                if ((FriendlyEnumEntryName.ToString() == InString) || (GetTextSourceString(FriendlyEnumEntryName) == InString))
                {
                    // Get the corresponding internal name and warn the user that we're using this fallback if not a user-defined enum
                    FString StringToImport = Enum->GetNameStringByIndex(EnumEntryIndex);
                    if (!Enum->IsA<UUserDefinedEnum>())
                    {
                        UE_LOG(LogAtelierSerialization, Warning, TEXT("Could not a find matching enum entry for '%s', but did find a matching display name. Will import using the enum entry corresponding to that display name ('%s')"), *InString, *StringToImport);
                    }
                    DoImport(StringToImport);
                    bNeedsImport = false;
                    break;
                }
            }
        }
    }
    #endif

    if (bNeedsImport)
    {
        DoImport(InString);
    }

    return true;
}

bool PropertySerializer::ImportString_Array(const FString& InString, const UArrayProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult)
{
    check(InProperty && InPropertyData);
    #if PROP_PORT_STRING_WITH_JSON
        if (InString.Len() >= 2 && InString[0] == '(' && InString[InString.Len() - 1] == ')')
        {
            FScriptArrayHelper ArrayHelper(InProperty, InPropertyData);

            // Trim the start('(') and end(')')
            FString StringToSplit = InString.Mid(1, InString.Len() - 2);
            #if 0
            TArray<FString> Values;
            StringToSplit.ParseIntoArray(Values, TEXT(","), true /* Cull Empty */);

            for (int32 Index = 0; Index < Values.Num(); ++Index)
            {
                int32 EntryIndex = ArrayHelper.AddValue();
                uint8* EntryData = ArrayHelper.GetRawPtr(EntryIndex);
                PropertySerializer::ImportString(Values[Index], InProperty->Inner, EntryData, InPortFlags, OutResult);
            }
            #else
            const TSharedRef< TJsonReader<TCHAR> > JsonReader = TJsonReaderFactory<TCHAR>::Create(StringToSplit);
            TSharedPtr<FJsonValue> JsonValue;
            if (!FJsonSerializer::Deserialize(JsonReader, JsonValue) || !JsonValue.IsValid())
            {
                OutResult.AddProblem(FString::Printf(TEXT("JsonObjectStringToUStruct - Unable to parse json=[%s]"), *InString));
                return false;
            }
            PropertySerializer::ImportJson(JsonValue.ToSharedRef(), InProperty, InPropertyData, OutResult);
            #endif
        }
        else
        {
            OutResult.AddProblem(FString::Printf(TEXT("%s - Malformed array string. It must start with '(' and end with ')'"), *InProperty->GetName()));
            return false;
        }
    #else
        UStructProperty* InnerStructProp = Cast<UStructProperty>(InProperty->Inner);
        if (InnerStructProp && InnerStructProp->Struct == FUserStructOnScope::StaticStruct())
        {
            SerializeUtils::AssignStringToProperty(InString, InProperty, InPropertyData, SerializeUtils::UseLuaString);
            return true;
            // return ExportLua(OutString, InProperty, InPropertyData);
        }
        return ImportString_Base(InString, InProperty, InPropertyData, InPortFlags, OutResult);
    #endif

    return true;
}

bool PropertySerializer::ImportString_Set(const FString& InString, const USetProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult)
{
    check(InProperty && InPropertyData);
    #if PROP_PORT_STRING_WITH_JSON
        // TODO: Not Complete
        check(InProperty && InPropertyData);
        if (InString.Len() >= 2 && InString[0] == '(' && InString[InString.Len() - 1] == ')')
        {
        }
    #else
        return ImportString_Base(InString, InProperty, InPropertyData, InPortFlags, OutResult);
    #endif

    return true;
}

bool PropertySerializer::ImportString_Map(const FString& InString, const UMapProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult)
{
    check(InProperty && InPropertyData);
    #if PROP_PORT_STRING_WITH_JSON
        // TODO: Not Complet
        check(InProperty && InPropertyData);
        if (InString.Len() >= 2 && InString[0] == '(' && InString[InString.Len() - 1] == ')')
        {
        }
    #else
        return ImportString_Base(InString, InProperty, InPropertyData, InPortFlags, OutResult);
    #endif
    return true;
}

bool PropertySerializer::ImportString_Struct(const FString& InString, const UStructProperty* InProperty, uint8* InPropertyData, const int32 InPortFlags, FSerializeResult& OutResult)
{
    check(InProperty && InPropertyData);
    #if PROP_PORT_STRING_WITH_JSON
        // TODO: Not Complet
    return StructSerializer::ImportJsonString(InString, InProperty->Struct, InPropertyData, OutResult);
    #else
        if (InProperty->Struct == FUserStructOnScope::StaticStruct())
        {
            if (FUserStructOnScope* StructPtr = (FUserStructOnScope*)(InPropertyData))
            {
                return StructSerializer::ImportLuaString(InString, InProperty->Struct, InPropertyData, OutResult);
            }
        }
        return ImportString_Base(InString, InProperty, InPropertyData, InPortFlags, OutResult);
    #endif

    return true;
}

bool PropertySerializer::ImportContainerEntryLua(const TSharedRef<FLuaValue>& InLuaValue, int32 EntryIndex, const FString& ContainerPropName, UProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult)
{
    FSerializeResult OutProblem;
    bool Result = PropertySerializer::ImportLua(InLuaValue, InProperty, InPropertyData, OutProblem);
    if (Result == false || OutProblem.HasError())
    {
        OutResult.AddProblem(FString::Printf(TEXT("Entry %d on property '%s' import error begin: "), EntryIndex, *ContainerPropName));
        for (FString& Problem : OutProblem.OutProblems)
        {
            OutResult.AddProblem(Problem);
        }
        OutResult.AddProblem(FString::Printf(TEXT("Entry %d on property '%s' import error end! "), EntryIndex, *ContainerPropName));
    }
    return Result;
}

bool PropertySerializer::ImportLua(const TSharedRef<FLuaValue>& InLuaValue, const UProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult)
{
    TYPED_IMPORT_LUA_AND_RETURN(Enum);
    TYPED_IMPORT_LUA_AND_RETURN(Numeric);
    TYPED_IMPORT_LUA_AND_RETURN(Bool);
    TYPED_IMPORT_LUA_AND_RETURN(Array);
    TYPED_IMPORT_LUA_AND_RETURN(Set);
    TYPED_IMPORT_LUA_AND_RETURN(Map);
    TYPED_IMPORT_LUA_AND_RETURN(Struct);

    return ImportLua_Base(InLuaValue, InProperty, InPropertyData, OutResult);
}

bool PropertySerializer::ImportLua_Base(const TSharedRef<FLuaValue>& InLuaValue, const UProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult)
{
    FString Error = SerializeUtils::AssignStringToProperty(InLuaValue->GetString(), InProperty, (uint8*)InPropertyData);
    if (!Error.IsEmpty())
    {
        OutResult.AddProblem(FString::Printf(TEXT("Property assigning string '%s' to property '%s' : %s "), *InLuaValue->GetString(), *InProperty->GetName(), *Error));
        return false;
    }

    return true;
}

bool PropertySerializer::ImportLua_Enum(const TSharedRef<FLuaValue>& InLuaValue, const UEnumProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult)
{
    if (InLuaValue->GetString().IsNumeric())
    {
        int64 EnumIntValue = InLuaValue->GetInt64();
        InProperty->GetUnderlyingProperty()->SetIntPropertyValue((void*)InPropertyData, EnumIntValue);
    }
    else
    {
        FString Error = SerializeUtils::AssignStringToProperty(InLuaValue->GetString(), InProperty, (uint8*)InPropertyData);
        if (!Error.IsEmpty())
        {
            OutResult.AddProblem(FString::Printf(TEXT("Property '%s' has invalid enum value: %s."), *InProperty->GetName(), *InLuaValue->GetString()));
            return false;
        }
    }

    return true;
}

bool PropertySerializer::ImportLua_Numeric(const TSharedRef<FLuaValue>& InLuaValue, const UNumericProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult)
{
    // Enum
    if (InProperty->IsEnum())
    {
        #ifdef SERIAL_ENUM_AS_NAME_STRING
            FString Error = SerializeUtils::AssignStringToProperty(InLuaValue->GetString(), InProperty, (uint8*)InPropertyData);
            if (!Error.IsEmpty())
            {
                OutResult.AddProblem(FString::Printf(TEXT("Property '%s' has invalid enum value: %s."), *InProperty->GetName(), *InLuaValue->GetString()));
                return false;
            }
        #else
            int64 EnumIntValue = InLuaValue->GetInt64();
            InProperty->SetIntPropertyValue((void*)InPropertyData, EnumIntValue);
        #endif
    }
    // Integer
    else if (InProperty->IsInteger())
    {
        int64 IntValue = InLuaValue->GetInt64();
        InProperty->SetIntPropertyValue((void*)InPropertyData, IntValue);
    }
    // Double
    else
    {
        double DoubleValue = InLuaValue->GetDouble();
        InProperty->SetFloatingPointPropertyValue((void*)InPropertyData, DoubleValue);
    }

    return true;
}

bool PropertySerializer::ImportLua_Bool(const TSharedRef<FLuaValue>& InLuaValue, const UBoolProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult)
{
    InProperty->SetPropertyValue((void*)InPropertyData, InLuaValue->GetBoolean());
    return true;
}

bool PropertySerializer::ImportLua_Array(const TSharedRef<FLuaValue>& InLuaValue, const UArrayProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult)
{
    TArray< TSharedPtr<FLuaValue> > ArrayValues;
    if (!InLuaValue->TryGetArray(ArrayValues))
    {
        OutResult.AddProblem(FString::Printf(TEXT("Property '%s' is the incorrect type. Expected Lua Array, got %s."), *InProperty->GetName(), *InLuaValue->GetString()));
        return false;
    }

    FScriptArrayHelper ArrayHelper(InProperty, InPropertyData);
    ArrayHelper.EmptyValues();
    for (const TSharedPtr<FLuaValue>& ValueEntry : ArrayValues)
    {
        const int32 NewEntryIndex = ArrayHelper.AddValue();
        uint8* EntryData = ArrayHelper.GetRawPtr(NewEntryIndex);
        PropertySerializer::ImportContainerEntryLua(ValueEntry.ToSharedRef(), NewEntryIndex, InProperty->GetName(), InProperty->Inner, EntryData, OutResult);
    }
    return true;
}

bool PropertySerializer::ImportLua_Set(const TSharedRef<FLuaValue>& InLuaValue, const USetProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult)
{
    return false;
}

bool PropertySerializer::ImportLua_Map(const TSharedRef<FLuaValue>& InLuaValue, const UMapProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult)
{
    return false;
}

bool PropertySerializer::ImportLua_Struct(const TSharedRef<FLuaValue>& InLuaValue, const UStructProperty* InProperty, const uint8* InPropertyData, FSerializeResult& OutResult)
{
    return StructSerializer::ImportLuaString(StaticCastSharedRef<FLuaTable>(InLuaValue), InProperty->Struct, (void*)InPropertyData, OutResult);
}
#pragma endregion Import

#pragma region Export
#define TYPED_EXPORT_STRING_AND_RETURN(Type) \
    if (const U##Type##Property* TypeProp = Cast<const U##Type##Property>(InProperty)) return ExportString_##Type(OutString, TypeProp, InPropertyData, InPortFlags)
#if WITH_EDITOR
bool PropertySerializer::ExportString(FString& OutString, const UProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags)
{
    TYPED_EXPORT_STRING_AND_RETURN(Array);
    TYPED_EXPORT_STRING_AND_RETURN(Set);
    TYPED_EXPORT_STRING_AND_RETURN(Map);
    TYPED_EXPORT_STRING_AND_RETURN(Struct);

    return ExportString_Base(OutString, InProperty, InPropertyData, InPortFlags);
}

bool PropertySerializer::ExportString_Base(FString& OutString, const UProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags)
{
    InProperty->ExportText_Direct(OutString, InPropertyData, InPropertyData, nullptr, InPortFlags);
    return true;
}

void WriteJSONArrayStartWithOptionalIdentifier(TSharedRef<FSerializeJsonWriter> InJsonWriter, const FString* InIdentifier)
{
    if (InIdentifier)
    {
        InJsonWriter->WriteArrayStart(*InIdentifier);
    }
    else
    {
        InJsonWriter->WriteArrayStart();
    }
}

void WriteJSONObjectStartWithOptionalIdentifier(TSharedRef<FSerializeJsonWriter> InJsonWriter, const FString* InIdentifier)
{
    if (InIdentifier)
    {
        InJsonWriter->WriteObjectStart(*InIdentifier);
    }
    else
    {
        InJsonWriter->WriteObjectStart();
    }
}

template <typename ValueType>
void WriteJSONValueWithOptionalIdentifier(TSharedRef<FSerializeJsonWriter> InJsonWriter, const FString* InIdentifier, const ValueType InValue)
{
    if (InIdentifier)
    {
        InJsonWriter->WriteValue(*InIdentifier, InValue);
    }
    else
    {
        InJsonWriter->WriteValue(InValue);
    }
}

/** generatic version */
bool PropertySerializer::ExportJson(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const UProperty* InProperty, void* InPropertyData)
{
    const FString Identifier = SerializeUtils::GetPropertyExportName(InProperty);
    return ExportJson(OutJsonWriter, &Identifier, InProperty, InPropertyData);
}

#define TYPED_EXPORT_JSON_AND_RETURN(Type) \
    if (const U##Type##Property* TypeProp = Cast<const U##Type##Property>(InProperty)) return ExportJson_##Type(OutJsonWriter, Identifier, TypeProp, InPropertyData)

bool PropertySerializer::ExportJson(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UProperty* InProperty, void* InPropertyData)
{
    TYPED_EXPORT_JSON_AND_RETURN(Enum);
    TYPED_EXPORT_JSON_AND_RETURN(Numeric);
    TYPED_EXPORT_JSON_AND_RETURN(Bool);
    TYPED_EXPORT_JSON_AND_RETURN(Array);
    TYPED_EXPORT_JSON_AND_RETURN(Set);
    TYPED_EXPORT_JSON_AND_RETURN(Map);
    TYPED_EXPORT_JSON_AND_RETURN(Struct);

    const FString PropValue = SerializeUtils::GetPropertyValueAsString(InProperty, (uint8*)InPropertyData);
    WriteJSONValueWithOptionalIdentifier(OutJsonWriter, Identifier, PropValue);
    return true;
}

/** For Enum */
bool PropertySerializer::ExportJson_Enum(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UEnumProperty* InProperty, void* InPropertyData)
{
    const FString PropValue = SerializeUtils::GetPropertyValueAsString(InProperty, (uint8*)InPropertyData);
    WriteJSONValueWithOptionalIdentifier(OutJsonWriter, Identifier, PropValue);
    return true;
}

/** For Number */
bool PropertySerializer::ExportJson_Numeric(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UNumericProperty* InProperty, void* InPropertyData)
{
    if (InProperty->IsEnum())
    {
        const FString PropValue = SerializeUtils::GetPropertyValueAsString(InProperty, (uint8*)InPropertyData);
        WriteJSONValueWithOptionalIdentifier(OutJsonWriter, Identifier, PropValue);
    }
    else if (InProperty->IsInteger())
    {
        const int64 PropValue = InProperty->GetSignedIntPropertyValue(InPropertyData);
        WriteJSONValueWithOptionalIdentifier(OutJsonWriter, Identifier, PropValue);
    }
    else
    {
        const double PropValue = InProperty->GetFloatingPointPropertyValue(InPropertyData);
        WriteJSONValueWithOptionalIdentifier(OutJsonWriter, Identifier, PropValue);
    }

    return true;
}

/** For Boolean */
bool PropertySerializer::ExportJson_Bool(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UBoolProperty* InProperty, void* InPropertyData)
{
    const bool PropValue = InProperty->GetPropertyValue(InPropertyData);
    WriteJSONValueWithOptionalIdentifier(OutJsonWriter, Identifier, PropValue);
    return true;
}

/** For Array */
bool PropertySerializer::ExportJson_Array(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UArrayProperty* InProperty, void* InPropertyData)
{
    WriteJSONArrayStartWithOptionalIdentifier(OutJsonWriter, Identifier);
    FScriptArrayHelper ArrayHelper(InProperty, InPropertyData);
    for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
    {
        const uint8* EntryData = ArrayHelper.GetRawPtr(Index);
        PropertySerializer::ExportJson(OutJsonWriter, nullptr, InProperty->Inner, (void*)EntryData);
    }
    OutJsonWriter->WriteArrayEnd();
    return true;
}

bool PropertySerializer::ExportJson_Set(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const USetProperty* InProperty, void* InPropertyData)
{
    WriteJSONArrayStartWithOptionalIdentifier(OutJsonWriter, Identifier);
    FScriptSetHelper SetHelper(InProperty, InPropertyData);
    for (int32 Index = 0; Index < SetHelper.GetMaxIndex(); ++Index)
    {
        if (SetHelper.IsValidIndex(Index))
        {
            const uint8* SetEntryData = SetHelper.GetElementPtr(Index);
            PropertySerializer::ExportJson(OutJsonWriter, nullptr, SetHelper.GetElementProperty(), (void*)SetEntryData);
        }
    }
    OutJsonWriter->WriteArrayEnd();
    return true;
}

bool PropertySerializer::ExportJson_Map(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UMapProperty* InProperty, void* InPropertyData)
{
    WriteJSONObjectStartWithOptionalIdentifier(OutJsonWriter, Identifier);
    FScriptMapHelper MapHelper(InProperty, InPropertyData);
    for (int32 MapSparseIndex = 0; MapSparseIndex < MapHelper.GetMaxIndex(); ++MapSparseIndex)
    {
        if (MapHelper.IsValidIndex(MapSparseIndex))
        {
            const uint8* MapKeyData = MapHelper.GetKeyPtr(MapSparseIndex);
            const uint8* MapValueData = MapHelper.GetValuePtr(MapSparseIndex);

            // JSON object keys must always be strings
            const FString KeyValue = SerializeUtils::GetPropertyValueAsStringDirect(MapHelper.GetKeyProperty(), (uint8*)MapKeyData);
            PropertySerializer::ExportJson(OutJsonWriter, &KeyValue, MapHelper.GetValueProperty(), (void*)MapValueData);
        }
    }
    OutJsonWriter->WriteObjectEnd();
    return true;
}

bool PropertySerializer::ExportJson_Struct(TSharedRef<FSerializeJsonWriter> OutJsonWriter, const FString* Identifier, const UStructProperty* InProperty, void* InPropertyData)
{
    WriteJSONObjectStartWithOptionalIdentifier(OutJsonWriter, Identifier);
    StructSerializer::ExportJson(OutJsonWriter, InProperty->Struct, InPropertyData);
    OutJsonWriter->WriteObjectEnd();
    return true;
}

bool PropertySerializer::ExportString_Array(FString& OutString, const UArrayProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags)
{
    check(InProperty && InPropertyData);
    #if PROP_PORT_STRING_WITH_JSON
        OutString.AppendChar('(');
        FString EntryStr;
        TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&EntryStr);
        PropertySerializer::ExportJson(JsonWriter, InProperty, const_cast<void*>((void*)InPropertyData));
        OutString.Append(EntryStr);

        OutString.AppendChar(')');
    #else
        UStructProperty* InnerStructProp = Cast<UStructProperty>(InProperty->Inner);
        if (InnerStructProp && InnerStructProp->Struct == FUserStructOnScope::StaticStruct())
        {
            return ExportLua(OutString, InProperty, InPropertyData);
        }
        else
        {
            return ExportString_Base(OutString, InProperty, InPropertyData, InPortFlags);
        }
    #endif

    return true;
}

bool PropertySerializer::ExportString_Set(FString& OutString, const USetProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags)
{
    check(InProperty && InPropertyData);
    #if PROP_PORT_STRING_WITH_JSON
        OutString.AppendChar('(');

        bool bExportAsJson = InProperty->ElementProp->IsA<UStructProperty>();
        int32 NumWrittenSetEntries = 0;
        FScriptSetHelper SetHelper(InProperty, InPropertyData);
        for (int32 Index = 0; Index < SetHelper.GetMaxIndex(); ++Index)
        {
            if (SetHelper.IsValidIndex(Index))
            {
                if (NumWrittenSetEntries++ > 0)
                {
                    OutString.AppendChar(',');
                    OutString.AppendChar(' ');
                }
                const uint8* EntryData = SetHelper.GetElementPtr(Index);
                FString EntryStr;
                #if 0
                if (bExportAsJson)
                {
                    const UStructProperty* StructInner = CastChecked<const UStructProperty>(InProperty->ElementProp);
                    StructSerializer::ExportStringWithTrim(EntryStr, StructInner->Struct, EntryData);
                }
                else
                {
                    InProperty->ExportText_Direct(EntryStr, InPropertyData, InPropertyData, nullptr, InPortFlags);
                }
                #else
                    PropertySerializer::ExportString(EntryStr, InProperty->ElementProp, EntryData, InPortFlags);
                #endif

                OutString.Append(EntryStr);
            }
        }

        OutString.AppendChar(')');
    #else
        return ExportString_Base(OutString, InProperty, InPropertyData, InPortFlags);
    #endif

    return true;
}

bool PropertySerializer::ExportString_Map(FString& OutString, const UMapProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags)
{
    check(InProperty && InPropertyData);
    #if PROP_PORT_STRING_WITH_JSON
        OutString.AppendChar('(');
        int32 NumWrittenSetEntries = 0;
        FScriptMapHelper MapHelper(InProperty, InPropertyData); 
        bool bExportAsJson = MapHelper.GetValueProperty()->IsA<UStructProperty>();
        for (int32 MapSparseIndex = 0; MapSparseIndex < MapHelper.GetMaxIndex(); ++MapSparseIndex)
        {
            if (MapHelper.IsValidIndex(MapSparseIndex))
            {
                if (NumWrittenSetEntries++ > 0)
                {
                    OutString.AppendChar(',');
                    OutString.AppendChar(' ');
                }

                const uint8* MapKeyData = MapHelper.GetKeyPtr(MapSparseIndex);
                const uint8* MapValueData = MapHelper.GetValuePtr(MapSparseIndex);

                FString KeyString, ValueString;

                // Export Key
                OutString.AppendChar('"');
                PropertySerializer::ExportString(KeyString, MapHelper.GetKeyProperty(), MapKeyData, InPortFlags);
                OutString.Append(KeyString);
                OutString.AppendChar('"');

                OutString.Append(TEXT(" = "));

                // Export Value
                #if 0
                if (bExportAsJson)
                {
                    const UStructProperty* StructInner = CastChecked<const UStructProperty>(MapHelper.GetValueProperty());
                    PropertySerializer::ExportString(ValueString, StructInner, MapValueData, InPortFlags);
                    OutString.Append(ValueString);
                }
                else
                {
                    PropertySerializer::ExportString(ValueString, MapHelper.GetValueProperty(), MapValueData, InPortFlags);
                    OutString.Append(ValueString);
                }
                #else
                PropertySerializer::ExportString(ValueString, MapHelper.GetValueProperty(), MapValueData, InPortFlags);
                OutString.Append(ValueString);
                #endif
            }
        }
        OutString.AppendChar(')');
    #else
        return ExportString_Base(OutString, InProperty, InPropertyData, InPortFlags);
    #endif

    return true;
}

bool PropertySerializer::ExportString_Struct(FString& OutString, const UStructProperty* InProperty, const uint8* InPropertyData, const int32 InPortFlags)
{
    check(InProperty && InPropertyData);
    #if PROP_PORT_STRING_WITH_JSON
        StructSerializer::ExportJsonStringWithTrim(OutString, InProperty->Struct, InPropertyData);
    #else
        if (InProperty->Struct == FUserStructOnScope::StaticStruct())
        {
            FLuaSerializeHelper::WriteLuaTableStartString(OutString);
            StructSerializer::ExportLuaString(OutString, InProperty->Struct, InPropertyData);
            FLuaSerializeHelper::WriteLuaTableEndString(OutString);
            return true;
        }
        return ExportString_Base(OutString, InProperty, InPropertyData, InPortFlags);
    #endif
    return true;
}

#define TYPED_EXPORT_LUA_AND_RETURN(Type) \
    if (const U##Type##Property* TypeProp = Cast<const U##Type##Property>(InProperty)) return ExportLua_##Type(OutString, TypeProp, InPropertyData)

bool PropertySerializer::ExportLua(FString& OutString, const UProperty* InProperty, const uint8* InPropertyData)
{
    TYPED_EXPORT_LUA_AND_RETURN(Enum);
    TYPED_EXPORT_LUA_AND_RETURN(Numeric);
    TYPED_EXPORT_LUA_AND_RETURN(Bool);
    TYPED_EXPORT_LUA_AND_RETURN(Array);
    TYPED_EXPORT_LUA_AND_RETURN(Set);
    TYPED_EXPORT_LUA_AND_RETURN(Map);
    TYPED_EXPORT_LUA_AND_RETURN(Struct);

    return ExportLua_Base(OutString, InProperty, InPropertyData);
}

bool PropertySerializer::ExportLua_Base(FString& OutString, const UProperty* InProperty, const uint8* InPropertyData)
{
    FString PropValue;
    if (ExportString(PropValue, InProperty, InPropertyData, PPF_None))
    {
        FLuaSerializeHelper::WritePropValue(OutString, InProperty, PropValue);
        return true;
    }

    return false;
}

bool PropertySerializer::ExportLua_Enum(FString& OutString, const UEnumProperty* InProperty, const uint8* InPropertyData)
{
    #ifdef SERIAL_ENUM_AS_NAME_STRING
    const FString PropValue = SerializeUtils::GetPropertyValueAsString(InProperty, (uint8*)InPropertyData);
    #else
    const uint64 PropValue = InProperty->GetUnderlyingProperty()->GetUnsignedIntPropertyValue(InPropertyData);
    #endif
    FLuaSerializeHelper::WritePropValue(OutString, InProperty, PropValue);
    return true;
}

bool PropertySerializer::ExportLua_Numeric(FString& OutString, const UNumericProperty* InProperty, const uint8* InPropertyData)
{
    if (InProperty->IsEnum())
    {
        #ifdef SERIAL_ENUM_AS_NAME_STRING
        const FString PropValue = SerializeUtils::GetPropertyValueAsString(InProperty, (uint8*)InPropertyData);
        #else
        const uint64 PropValue = InProperty->GetSignedIntPropertyValue(InPropertyData);
        #endif
        FLuaSerializeHelper::WritePropValue(OutString, InProperty, PropValue);
    }
    else if (InProperty->IsInteger())
    {
        const int64 PropValue = InProperty->GetSignedIntPropertyValue(InPropertyData);
        FLuaSerializeHelper::WritePropValue(OutString, InProperty, PropValue);
    }
    else
    {
        const double PropValue = InProperty->GetFloatingPointPropertyValue(InPropertyData);
        FLuaSerializeHelper::WritePropValue(OutString, InProperty, PropValue);
    }

    return true;
}

bool PropertySerializer::ExportLua_Bool(FString& OutString, const UBoolProperty* InProperty, const uint8* InPropertyData)
{
    const bool PropValue = InProperty->GetPropertyValue(InPropertyData);
    FLuaSerializeHelper::WritePropValue(OutString, InProperty, PropValue);
    return true;
}

bool PropertySerializer::ExportLua_Array(FString& OutString, const UArrayProperty* InProperty, const uint8* InPropertyData)
{
    FLuaSerializeHelper::WriteLuaTableStartString(OutString);
    FScriptArrayHelper ArrayHelper(InProperty, InPropertyData);
    for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
    {
        const uint8* EntryData = ArrayHelper.GetRawPtr(Index);
        PropertySerializer::ExportLua(OutString, InProperty->Inner, EntryData);
        OutString += ",";
    }
    FLuaSerializeHelper::WriteLuaTableEndString(OutString);
    return true;
}

bool PropertySerializer::ExportLua_Set(FString& OutString, const USetProperty* InProperty, const uint8* InPropertyData)
{
    //TODO: Need Implementation
    return false;
}

bool PropertySerializer::ExportLua_Map(FString& OutString, const UMapProperty* InProperty, const uint8* InPropertyData)
{
    //TODO: Need Implementation
    return false;
}

bool PropertySerializer::ExportLua_Struct(FString& OutString, const UStructProperty* InProperty, const uint8* InPropertyData)
{
    check(InProperty && InPropertyData);
    FLuaSerializeHelper::WriteLuaTableStartString(OutString);
    StructSerializer::ExportLuaString(OutString, InProperty->Struct, InPropertyData);
    FLuaSerializeHelper::WriteLuaTableEndString(OutString);
    return true;

}
#endif
#pragma endregion Export

#pragma endregion