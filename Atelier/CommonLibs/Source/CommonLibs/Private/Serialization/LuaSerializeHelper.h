#pragma once
#include "CoreMinimal.h"
#include "UObject/UnrealType.h"

const FString LUA_USER_STRUCT_TYPE_PROP = "__ustruct";
const FString LUA_PROP_SPLIT = ",";

struct FLuaTable;
#if 0
typedef char CharType;
#else
typedef TCHAR CharType;
#endif

class FLuaSerializeHelper
{
public:

    static bool IsLuaTable(const FString& InString)
    {
        return InString.StartsWith("{") && InString.EndsWith("}");
    }

#if WITH_EDITOR
    static void WriteLuaTableStartString(FString& OutString)
    {
        OutString += "{";
    }

    static void WriteLuaTableEndString(FString& OutString)
    {
        OutString += "}";
    }

    static void WriteLuaPropStartString(FString& OutString, const UProperty* InProperty)
    {
        const FString Identifier = SerializeUtils::GetPropertyExportName(InProperty);
        if (!OutString.IsEmpty() && !OutString.EndsWith("{")) OutString += LUA_PROP_SPLIT;
        OutString += Identifier;
        OutString += "=";
    }

    template <typename T>
    static void WritePropValue(FString& OutString, const UProperty* InProperty, const T& PropValue)
    {
        const FString Identifier = SerializeUtils::GetPropertyExportName(InProperty);
        WriteValue(OutString, PropValue);
    }

private:
    template <typename T>
    static void WriteValue(FString& OutString, const T& Value) {}
    static void WriteValue(FString& OutString, const int32& Value) { OutString += FString::Printf(TEXT("%d"), Value); }
    static void WriteValue(FString& OutString, const int64& Value) { OutString += FString::Printf(TEXT("%d"), Value); }
    static void WriteValue(FString& OutString, const uint64& Value) { OutString += FString::Printf(TEXT("%d"), Value); }
    static void WriteValue(FString& OutString, const float& Value) { OutString += FString::Printf(TEXT("%g"), Value); }
    static void WriteValue(FString& OutString, const double& Value) { OutString += FString::Printf(TEXT("%.17g"), Value); }
    static void WriteValue(FString& OutString, const FString& Value) { OutString += "'" + Value + "'"; }
    static void WriteValue(FString& OutString, const bool& Value) { OutString += Value ? "1" : "0"; }
#endif
};

struct FLuaValue
{
public:
    FLuaValue() {}
    FLuaValue(const FString& InString) : ValueString(InString) {}
    virtual ~FLuaValue() {}

    FString GetString() { return ValueString; }
    int64 GetInt64() { return FCString::Atoi64(*ValueString); }
    double GetDouble() { return FCString::Atod(*ValueString); }
    bool GetBoolean() { return FCString::ToBool(*ValueString); }
    virtual bool TryGetArray(TArray<TSharedPtr<FLuaValue>>& OutArray) { return false; }

    FString ValueString;
};

struct FLuaProp
{
public:
    ~FLuaProp()
    {
        Value = nullptr;
    }

    bool IsAnonymouse() { return Name.IsEmpty(); }

    FString Name;
    TSharedPtr<FLuaValue> Value;
};

struct FLuaTable : public FLuaValue
{
public:
    FLuaTable() {}
    FLuaTable(const FString& InString) : FLuaValue(InString)
    {}
    TMap<FString, TSharedPtr<FLuaProp>> Props;
    TArray<TSharedPtr<FLuaValue>> ArrayValues;

    TSharedPtr<FLuaProp> GetPropByName(const FString& Name)
    {
        if (Props.Num() <= 0) return nullptr;

        if (Props.Contains(Name)) return Props[Name];
        return nullptr;
    }

    virtual bool TryGetArray(TArray<TSharedPtr<FLuaValue>>& OutArray) override 
    { 
        OutArray.Empty();
        for (auto Item : ArrayValues)
        {
            OutArray.Add(Item);
        }

        return true;
    }
};

template <typename ElementType>
struct TSimpleStack
{
public:
    void Push(const ElementType& Item) { Array.Push(Item); }
    ElementType& Top() { return Array.Top(); }
    ElementType Pop() { return Array.Pop(); }
    int32 Num() { return Array.Num(); }
    bool IsEmpty() { return Num() <= 0; }
private:
    TArray<ElementType> Array;
};

typedef TSimpleStack<CharType> FReaderStack;

struct FLuaReader
{
public:
    TSharedPtr<FLuaValue> LuaValue;

    FLuaReader(const FString& InString)
    {
        LuaValue = MakeShareable(new FLuaTable());
        LoadLuaString(InString);
    }

private:
    class FSingleLoader
    {
    public:
        FReaderStack OpenStack;

        FSingleLoader() {}
        virtual ~FSingleLoader() {}

        virtual void LoadSingleChar(CharType Ch) 
        {}

        virtual void FinishLoad() {}
    };


    class FValueLoaderImpl : public FSingleLoader
    {
    public:
        virtual TSharedPtr<FLuaValue> GetValue() = 0;
        virtual bool IsTableLoader() { return false; }
    };

    class FCommomValueLoader : public FValueLoaderImpl
    {
    public:
        FString ValueString;
        TSharedPtr<FLuaValue> LuaValue;

        FCommomValueLoader()
            : LuaValue(nullptr)
        {}
        
        virtual void LoadSingleChar(CharType Ch) override
        {
            ValueString.AppendChar(Ch);
        }

        virtual void FinishLoad() override
        {
            if (ValueString.StartsWith("'")) ValueString.RemoveAt(0);
            if (ValueString.EndsWith("'")) ValueString.RemoveAt(ValueString.Len() - 1);
            if (ValueString.StartsWith("\"")) ValueString.RemoveAt(0);
            if (ValueString.EndsWith("\"")) ValueString.RemoveAt(ValueString.Len() - 1);
            LuaValue = MakeShareable(new FLuaValue(ValueString));
        }

        TSharedPtr<FLuaValue> GetValue() override { return LuaValue; }
    };

    class FPropLoader;
    class FTableLoader : public FValueLoaderImpl
    {
    public:
        TSharedPtr<FPropLoader> InnerPropLoader;
        TSharedPtr<FLuaTable> LuaTable;
        FTableLoader()
            : InnerPropLoader(nullptr)
        {
            LuaTable = MakeShareable(new FLuaTable());
        }

        virtual bool IsTableLoader() override { return true; }
        virtual void LoadSingleChar(CharType Ch) override
        {
            LuaTable->ValueString.AppendChar(Ch);
            if (Ch == '{') 
            {
                // 本Table的起始{不需要往下传递
                if (!OpenStack.IsEmpty())
                {
                    PostInnerPropLoadChar(Ch);
                }
                OpenStack.Push(Ch);
            }
            else if (Ch == '}' && !OpenStack.IsEmpty() && OpenStack.Top() == '{') 
            {
                // 本Table的结束}不需要往下传递
                if (OpenStack.Num() > 1)
                {
                    PostInnerPropLoadChar(Ch);
                }
                OpenStack.Pop();
            }
            else if (Ch == '\'' || Ch == '"') 
            {
                if (!OpenStack.IsEmpty() && OpenStack.Top() == Ch)
                {
                    OpenStack.Pop();
                }
                else
                {
                    OpenStack.Push(Ch);
                }
                PostInnerPropLoadChar(Ch);
            }
            else if (Ch == ',' && OpenStack.Num() <= 1)
            {
                PostInnerPropLoaded();
            }
            else
            {
                PostInnerPropLoadChar(Ch);
            }
        }

         virtual void FinishLoad() override
         {
            if (InnerPropLoader.IsValid()) PostInnerPropLoaded();
         }

        void PostInnerPropLoadChar(CharType Ch)
        {
            if (!InnerPropLoader.IsValid())
            {
                InnerPropLoader = MakeShareable(new FPropLoader());
            }
            InnerPropLoader->LoadSingleChar(Ch);
        }

        void PostInnerPropLoaded()
        {
            if (InnerPropLoader)
            {
                InnerPropLoader->FinishLoad();
                if (InnerPropLoader->LuaProp->IsAnonymouse())
                {
                    LuaTable->ArrayValues.Add(InnerPropLoader->LuaProp->Value);
                }
                else
                {
                    LuaTable->Props.Add(InnerPropLoader->LuaProp->Name, InnerPropLoader->LuaProp);
                }

                InnerPropLoader = nullptr;
            }
        }

        TSharedPtr<FLuaValue> GetValue() override { return LuaTable; }
    };

    class FValueLoader : public FSingleLoader
    {
    public:
        FValueLoaderImpl* LoaderImpl;

        FValueLoader()
            : LoaderImpl(nullptr)
        {}

        ~FValueLoader()
        {
            delete LoaderImpl;
            LoaderImpl = nullptr;
        }

        virtual void LoadSingleChar(CharType Ch) override
        {
            bool bValueNotStart = LoaderImpl == nullptr;
            if (bValueNotStart)
            {
                // 过滤空格
                if (Ch == ' ') return ;

                if (Ch == '{')
                    LoaderImpl = new FTableLoader();
                else 
                    LoaderImpl = new FCommomValueLoader();
            }

            LoaderImpl->LoadSingleChar(Ch);
        }

        virtual void FinishLoad() override
        {
            if (LoaderImpl) LoaderImpl->FinishLoad();
        }

        TSharedPtr<FLuaValue> GetValue() { return LoaderImpl ? LoaderImpl->GetValue() : nullptr; }
    };

    class FPropLoader : public FSingleLoader
    {
    public:
        int32 ValueLoaderIndex;
        FValueLoader ValueLoaders[2];
        TSharedPtr<FLuaProp> LuaProp;

        FPropLoader()
            : FSingleLoader()
            , ValueLoaderIndex(0)
        {
            LuaProp = MakeShareable(new FLuaProp());
        }

        virtual void LoadSingleChar(CharType Ch) override
        {
            if (Ch == '\'' || Ch == '"')
            {
                if (!OpenStack.IsEmpty() && OpenStack.Top() == Ch) OpenStack.Pop();
                else OpenStack.Push(Ch);
                GetValueLoader().LoadSingleChar(Ch);
            }
            else if (Ch == '=')
            {
                // 需确定这个'='是本Prop处理还是下层的Value处理
                // 有可能存在以下几种情况
                //      prop自身有key, '=' 是本prop的赋值号, 此时 ValueLoaderIndex == 0
                //      prop自身有key, '=' 是本value里面的符号, 此时 ValueLoaderIndex == 1
                //      prop自身无key， '=' 是value里面的符号, 此时 ValueLoaderIndex == 0, 且对应Loader一定是Table 或者 String
                if (ValueLoaderIndex == 0)
                {
                    // 本Prop是一个Array的元素，且元素值为String
                    if (!OpenStack.IsEmpty())
                    {
                        GetValueLoader().LoadSingleChar(Ch);
                    }
                    else if (GetValueLoader().LoaderImpl->IsTableLoader())
                    {
                        GetValueLoader().LoadSingleChar(Ch);
                    }
                    else
                    {
                        StepLoaderIndex();
                    }
                }
                else if (ValueLoaderIndex == 1)
                {
                    GetValueLoader().LoadSingleChar(Ch);
                }
            }
            else
            {
                GetValueLoader().LoadSingleChar(Ch);
            }
        }

        virtual void FinishLoad() override
        {
            // 如果只有一个值，那么该prop匿名，比如 Array中的项
            if (ValueLoaderIndex == 0)
            {
                ValueLoaders[0].FinishLoad();
                LuaProp->Value = ValueLoaders[0].GetValue();
            }
            else
            {
                ValueLoaders[0].FinishLoad();
                LuaProp->Name = ValueLoaders[0].GetValue()->GetString().TrimStartAndEnd();

                ValueLoaders[1].FinishLoad();
                LuaProp->Value = ValueLoaders[1].GetValue();
            }
        }

        void StepLoaderIndex() { ValueLoaderIndex = FMath::Min(ValueLoaderIndex + 1, 1); }
        FValueLoader& GetValueLoader() { return ValueLoaders[ValueLoaderIndex]; }
    };

    void LoadLuaString(const FString& InString)
    {
        FPropLoader Loader;

        // CharType* RawChars = TCHAR_TO_UTF8(*InString);
        TArray<CharType> RawChars = InString.GetCharArray();
        // TArray<TCHAR>& FixedCharArray = FixedString.GetCharArray();
        for (int32 Index = 0; Index < InString.Len(); Index++)
        {
            CharType Ch = RawChars[Index];
            Loader.LoadSingleChar(Ch);
        }

        Loader.FinishLoad();

        LuaValue = Loader.LuaProp->Value;
    }
};