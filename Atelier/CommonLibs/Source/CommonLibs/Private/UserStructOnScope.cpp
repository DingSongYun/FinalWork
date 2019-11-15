#include "UserStructOnScope.h"
#include "DataTableUtils.h"
#include "Serialization/SerializeUtils.h"
#include "AssetData.h"
#include "AssetRegistryModule.h"

#define GET_PROP_VALUE(Prop) \
	Prop->GetPropertyValue(Prop->ContainerPtrToValuePtr<void>(InnerStruct->GetStructMemory()))

#define SET_PROP_VALUE(Prop, InValue) \
	Prop->SetPropertyValue(Prop->ContainerPtrToValuePtr<void>(InnerStruct->GetStructMemory()), InValue)

FUserStructOnScope::FUserStructOnScope()
#if WITH_EDITOR
	: bFixedStructType(false)
	, UserStructBaseType(UScriptStruct::StaticClass())
#endif
{}

void FUserStructOnScope::Initialize(const UScriptStruct& UserStruct)
{
	InnerStruct = MakeShareable(new FStructOnScope(&UserStruct));
}

void FUserStructOnScope::Reset()
{
	if (IsValid())
	{
		InnerStruct->Reset();
	}

	PropDict.Empty();
}

UProperty* FUserStructOnScope::GetStructPropertyByName(const FName& InName)
{
	const UStruct* StructType = InnerStruct->GetStruct();
#if false
	UProperty* Property = StructType->FindPropertyByName(InName);

	return Property;
#else
	if (PropDict.Contains(InName)) return PropDict[InName];

	if (StructType->IsNative())
	{
		UProperty* Ret = StructType->FindPropertyByName(InName);
		PropDict.Add(InName, Ret);
		return Ret;
	}

	FString PropName = InName.ToString();
	for (UProperty* Property = StructType->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
	{
		FString FieldName = Property->GetName();
		if (FieldName.StartsWith(PropName, ESearchCase::CaseSensitive))
		{
			int Index = FieldName.Len();
			for (int i = 0; i < 2; ++i)
			{
				int FindIndex = FieldName.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, Index);
				if (FindIndex != INDEX_NONE)
				{
					Index = FindIndex;
				}
			}

			if (PropName.Len() == Index)
			{
				PropDict.Add(InName, Property);
				return Property;
			}
		}
	}
#endif

	return nullptr;
}

void FUserStructOnScope::SetStructPropertyValue(const FName& PropName, int32 InValue)
{
	if(UIntProperty* Prop = Cast<UIntProperty>(GetStructPropertyByName(PropName)))
	{
		SET_PROP_VALUE(Prop, InValue);
	}
}

void FUserStructOnScope::InnerImport()
{
	if(UScriptStruct* UserStruct = Cast<UScriptStruct>(UserStructPath.TryLoad()))
	{
		Initialize(*UserStruct);
		// FSerializeResult OutResult;
		// StructSerializer::ImportString(SerializeOnString, UserStruct, InnerStruct->GetStructMemory(), PPF_None, UserStruct->GetName(), OutResult);
	}
}

// bool FUserStructOnScope::Serialize(FArchive& Ar)
// {
// 	if (Ar.IsSaving())
// 	{
// 		Ar << InnerStruct;
// 	}
// 	return true;
// }

bool FUserStructOnScope::ExportTextItem(FString& ValueStr, FUserStructOnScope const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const
{
	if (IsValid())
	{
		if (const UScriptStruct* UserStruct = Cast<UScriptStruct>(InnerStruct->GetStruct()))
		{
			FSoftObjectPath StructPath(UserStruct);
			FString StructString;
			StructSerializer::ExportString(StructString, UserStruct, InnerStruct->GetStructMemory(), PPF_None);

			ValueStr += StructPath.ToString() + "|" + StructString;

			return true;
		}
	}

	return false;
}

bool FUserStructOnScope::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
{
	FString StructPath, StructString;
	if (FString(Buffer).Split("|", &StructPath, &StructString))
	{
		#if false
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*StructPath);
		if(UScriptStruct* UserStruct = Cast<UScriptStruct>(AssetData.GetAsset()))
		#else
		if(UScriptStruct* UserStruct = Cast<UScriptStruct>(FSoftObjectPath(StructPath).TryLoad()))
		#endif
		{
			UserStruct->GetLinker()->Preload(UserStruct);
			Initialize(*UserStruct);

			if (IsValid())
			{
				FSerializeResult OutResult;
				StructSerializer::ImportString(StructString, UserStruct, InnerStruct->GetStructMemory(), PPF_None, UserStruct->GetName(), OutResult);

				return true;
			}
		}
	}

	return false;
}

#if WITH_EDITOR
void FUserStructOnScope::InnerExport()
{
	if (IsValid())
	{
		if (const UScriptStruct* UserStruct = Cast<UScriptStruct>(InnerStruct->GetStruct()))
		{
			UserStructPath = FSoftObjectPath(UserStruct);
			// SerializeOnString = FString("");
			// StructSerializer::ExportString(SerializeOnString, UserStruct, InnerStruct->GetStructMemory(), PPF_None);
		}
	}
}
#endif

// FArchive& operator<<(class FArchive& Ar, TSharedPtr<FStructOnScope>& UserStructOnScope)
// {
// 	UScriptStruct* UserStruct = nullptr;
// 	FString StructString;
// 	if (Ar.IsLoading())
// 	{
// 		Ar << UserStruct;
// 		if (UserStruct)
// 		{
// 			UserStructOnScope = MakeShareable(new FStructOnScope(UserStruct));
// 		}

// 		Ar << StructString;
// 		if (UserStructOnScope.IsValid())
// 		{
// 			FSerializeResult OutResult;
// 			StructSerializer::ImportString(StructString, UserStruct, UserStructOnScope->GetStructMemory(), PPF_None, UserStruct->GetName(), OutResult);
// 		}
// 	}

// 	if (Ar.IsSaving())
// 	{
// 		if (UserStructOnScope.IsValid())
// 		{
// 			UserStruct = Cast<UScriptStruct>(const_cast<UStruct*>(UserStructOnScope->GetStruct()));
// 		}
// 		Ar << UserStruct;

// 		StructSerializer::ExportString(StructString, UserStruct, UserStructOnScope->GetStructMemory(), PPF_None);
// 		Ar << StructString;
// 	}

// 	return Ar;
// }