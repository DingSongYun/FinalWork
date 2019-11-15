// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-19

#include "ActionSkill.h"
#include "Modules/ModuleManager.h"
#include "Settings/ActionSkillSettings.h"
#include "UObject/StructOnScope.h"
#include "Engine/EngineTypes.h"
#include "MessageDialog.h"
#include "Animation/AnimSequenceBase.h"
#include "ActionSkillTableCSV.h"
#include "FileHelper.h"

DEFINE_LOG_CATEGORY(LogActionSkill);

#define LOCTEXT_NAMESPACE "FActionSkillModule"
#if WITH_EDITOR
int32 NextValidId = 0;
#endif

const FName SKILL_PROP_ID = "Id";
const FName SKILL_PROP_NAME = "Name";
const FName SKILL_PROP_ACTIONS = "Actions";

#define GET_PROP_VALUE(Prop) \
	Prop->GetPropertyValue(Prop->ContainerPtrToValuePtr<void>(InnerStruct->GetStructMemory()))

#define SET_PROP_VALUE(Prop, InValue) \
	Prop->SetPropertyValue(Prop->ContainerPtrToValuePtr<void>(InnerStruct->GetStructMemory()), InValue)
	
/*********************************************************************/
// FAction
/*********************************************************************/
float FAction::GetActionLength()
{
	if (!AnimReference.IsNull())
	{
		if (!AnimReference.IsValid())
		{
			AnimReference.LoadSynchronous();
		}
		return AnimReference->SequenceLength;
	}

	return 0;
}

float FAction::GetActionFrame()
{
	if (!AnimReference.IsNull())
	{
		if (!AnimReference.IsValid())
		{
			AnimReference.LoadSynchronous();
		}
		return AnimReference->GetNumberOfFrames();
	}

	return 0;
}

/*********************************************************************/
// FActionSkillScope
/*********************************************************************/
void FActionSkillScope::Initialize(UScriptStruct& SkillStruct)
{
	// Allocate struct memory from FStructOnScope, and make the scop owns the memory
	InnerStruct = MakeShareable(new FStructOnScope(&SkillStruct/*struct type info*/));
}

UProperty* FActionSkillScope::GetSkillPropertyByName(const FName& InName)
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

FString FActionSkillScope::GetName()
{
	if (UStrProperty* Prop = Cast<UStrProperty>(GetSkillPropertyByName(SKILL_PROP_NAME)))
	{
		return GET_PROP_VALUE(Prop);
	}

	return FString();
}

float FActionSkillScope::GetSkillActionStartPos(int32 Index)
{
	float length = 0.f;
	int i = 0;
	for (FActionIterator It = CreateActionIterator(); It; It++)
	{
		if (i == Index)
		{
			break;
		}
		length += It->GetActionLength();
		i++;
	}
	return length;
}

float FActionSkillScope::GetSkillActionLength(int32 Index)
{
	float length = 0.f;
	int i = 0;
	for (FActionIterator It = CreateActionIterator(); It; It++)
	{
		if (i == Index)
		{
			length = It->GetActionLength();
			break;
		}
		i++;
	}
	return length;
}

float FActionSkillScope::CalculateSkillLength()
{
	float CalculatedLength = 0.f;
	for (FActionIterator It = CreateActionIterator(); It; It++)
	{
		CalculatedLength += It->GetActionLength();
	}

	return CalculatedLength;
}

float FActionSkillScope::CalculateSkillFrame()
{
	float CalculatedFrame = 0.f;
	for (FActionIterator It = CreateActionIterator(); It; It++)
	{
		CalculatedFrame += It->GetActionFrame();
	}

	return CalculatedFrame;
}

int32 FActionSkillScope::GetId()
{
	if(UIntProperty* Prop = Cast<UIntProperty>(GetSkillPropertyByName(SKILL_PROP_ID)))
	{
		return GET_PROP_VALUE(Prop);
	}

	return 0;
}

FActionIterator FActionSkillScope::CreateActionIterator()
{
	if(UArrayProperty* Prop = Cast<UArrayProperty>(GetSkillPropertyByName(SKILL_PROP_ACTIONS)))
	{
		const FScriptArray& Array = GET_PROP_VALUE(Prop);
		return FActionIterator(const_cast<FScriptArray*>(&Array), Prop->Inner->ElementSize);
	}

	check(TEXT("Skill actions missed!!!"));
	return FActionIterator(nullptr, 0);
}

#if WITH_EDITOR
void FActionSkillScope::SetName(const FString& InName)
{
	if(UStrProperty* Prop = Cast<UStrProperty>(GetSkillPropertyByName(SKILL_PROP_NAME)))
	{
		SET_PROP_VALUE(Prop, InName);
	}
}

void FActionSkillScope::SetId(int32 InId)
{
	if(UIntProperty* Prop = Cast<UIntProperty>(GetSkillPropertyByName(SKILL_PROP_ID)))
	{
		SET_PROP_VALUE(Prop, InId);
	}
}

FAction& FActionSkillScope::NewAction()
{
	if(UArrayProperty* Prop = Cast<UArrayProperty>(GetSkillPropertyByName(SKILL_PROP_ACTIONS)))
	{
		FScriptArrayHelper ScriptArrayHelper(Prop, &GET_PROP_VALUE(Prop));
		int32 IndexToInitialize = ScriptArrayHelper.AddValue();
		void* ElementPtr = (void*)ScriptArrayHelper.GetRawPtr(IndexToInitialize);
		FAction* Ret = (FAction*)ElementPtr;
		MarkDirty(true);
		return *Ret;
	}

	check(TEXT("Add Action Failed !!!"));
	return *(new FAction());
}

void FActionSkillScope::RemoveAction(FAction& InAction)
{
	for (FActionIterator It = CreateActionIterator(); It; ++It)
	{
		if (*It == InAction)
		{
			if (UArrayProperty* Prop = Cast<UArrayProperty>(GetSkillPropertyByName(SKILL_PROP_ACTIONS)))
			{
				Prop->Inner->DestroyValue(It.GetItemRawPtr());
				It.RemoveCurrent();
				break;
			}
		}
	}
	MarkDirty(true);
}

void FActionSkillScope::MarkDirty(bool bIsDirty, UProperty* DirtyProperty)
{
	if (bDirty != bIsDirty)
	{
		bDirty = bIsDirty;
		OnSkillDataDirty.ExecuteIfBound(bDirty);
	}

	if (bIsDirty)
	{
		if (DirtyProperty)
		{
			/** Id变化需通知Skilltable */
			if (DirtyProperty == GetSkillPropertyByName(SKILL_PROP_ID))
			{
				OnSkillIdChanged.ExecuteIfBound(*this);
			}

			if (DirtyProperty->GetFName() == "EventId")
			{
				OnSelectedEventIdChanged.ExecuteIfBound();
			}
		}
	}
}
#endif

/*********************************************************************/
// UActionSkillTable
/*********************************************************************/
UActionSkillTable::UActionSkillTable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UActionSkillTable::EmptyTable()
{
	ActionSkills.Empty();
}

UScriptStruct& UActionSkillTable::GetSkillStruct()
{
	static const UActionSkillSettings* ActionSkillSetting = GetDefault<UActionSkillSettings>();
	if (ActionSkillSetting->SkillDefinitionStruct)
	{
		return *(ActionSkillSetting->SkillDefinitionStruct);
	}

	return *FActionSkillTemplate::StaticStruct();
}

UScriptStruct& UActionSkillTable::GetSubEventStruct()
{
	const TCHAR* SUB_EVENT_STRUCT_PATH = TEXT("/ActionSkill/ASSubEvent.ASSubEvent");
	static UScriptStruct* SubEventStructType = FindObject<UScriptStruct>(ANY_PACKAGE, SUB_EVENT_STRUCT_PATH);
	if (!SubEventStructType)
	{
		SubEventStructType = Cast<UScriptStruct>(StaticLoadObject(UObject::StaticClass(), nullptr, SUB_EVENT_STRUCT_PATH, nullptr, LOAD_None, nullptr, true));
	}

	// return UScriptStruct::StaticClass();
	check(SubEventStructType);
	return *SubEventStructType;
}

#if WITH_EDITOR

TSharedPtr<FActionSkillScope> UActionSkillTable::NewActionSkill()
{
	FActionSkillPtr NewASkill = MakeShareable(new FActionSkillScope());
	NewASkill->Initialize(UActionSkillTable::GetSkillStruct());
	NewASkill->OnSkillIdChanged.BindUObject(this, &UActionSkillTable::OnSkillIdChanged);
	NewASkill->OnSelectedEventIdChanged.BindUObject(this, &UActionSkillTable::OnSelectedEventIdChanged);
	NewASkill->NewAction();
	NewASkill->SetName(FString::Printf(TEXT("NewSkill_%d"), ActionSkills.Num()));
	NewASkill->SetId(NextValidId);
	UpdateNextValideSkillId(NextValidId);
	NewASkill->MarkDirty(true);

	ActionSkills.Add(NewASkill->GetId(), NewASkill);
	return NewASkill;
}

void UActionSkillTable::OnSkillIdChanged(FActionSkillScope& InSkill)
{
	int32 OldId = INDEX_NONE;
	int32 NewId = InSkill.GetId();

	for (SkillIterator It = CreateIterator(); It; ++It)
	{
		if (It->Value.Get() == &InSkill && It->Key != NewId)
		{
			OldId = It->Key;
			break;
		}
	}

	if(OldId != INDEX_NONE)
	{

		if (ActionSkills.Contains(NewId))
		{
			EAppReturnType::Type ReturnValue = FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(
						FString::Printf(TEXT("Duplicated Id (%d) on skills {%s, %s}"), 
							NewId,
							*ActionSkills.Find(NewId)->Get()->GetName(),
							*InSkill.GetName()
						))
			);

			InSkill.SetId(OldId);
			return;	
		}

		FActionSkillPtr SkillToMove;
		if (ActionSkills.RemoveAndCopyValue(OldId, SkillToMove))
		{
			ActionSkills.Add(NewId, SkillToMove);
		}
	}

	UpdateNextValideSkillId(NewId);
	return;
}

void UActionSkillTable::OnSelectedEventIdChanged()
{
	for (TMap<int32, FActionSkillPtr>::TConstIterator iter = ActionSkills.CreateConstIterator(); iter; ++iter)
	{
		FActionSkillScope skillscope = *iter->Value.Get();
		TArray<FActionKeyFrame>  keys = skillscope.CreateActionIterator()->Keys;
		for (FActionKeyFrame key : keys) 
		{
			FActionEventPtr ActionEventPtr = FActionSkillModule::Get().GetActionEventById(key.EventId);
			if(!ActionEventPtr.IsValid())
			{
				EAppReturnType::Type ReturnValue = FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(
					FString::Printf(TEXT("Id：(%d) event not exit"),key.EventId)));
			}
		}
	}
}

void UActionSkillTable::DeleteActionSkill(FActionSkillPtr InActionSkill)
{
	ActionSkills.Remove(InActionSkill->GetId());
}

void UActionSkillTable::DeleteActionEvent(int32 Id)
{
	for (TMap<int32, FActionSkillPtr>::TConstIterator iter = ActionSkills.CreateConstIterator(); iter; ++iter)
	{
		FActionSkillScope skillscope = *iter->Value.Get();
		//TArray<FActionKeyFrame>  keys = skillscope.CreateActionIterator()->Keys;
		int32 Index = skillscope.CreateActionIterator()->Keys.IndexOfByPredicate([&](const FActionKeyFrame& InKeyFrame) { return InKeyFrame.EventId == Id; });
		if (Index != INDEX_NONE)
		{
			skillscope.CreateActionIterator()->Keys.RemoveAt(Index);
		}
	}
}

void UActionSkillTable::OnSaveData()
{
	for (auto It = CreateConstIterator(); It; ++It)
	{
		It->Value->MarkDirty(false);
	}
}

void UActionSkillTable::UpdateNextValideSkillId(int32 NewId)
{
	if (NewId >= NextValidId)
	{
		NextValidId = NewId + 1;
	}
}
#endif

/*********************************************************************/
// FActionSkillModule
/*********************************************************************/
void FActionSkillModule::RegisterActionKeyNotifyHandler(const UStruct* Struct, FOnActionKeyEventNotified Handler)
{
	if (!Struct) return ;
	RegisteredActionKeyNotifyHandler.Add(Struct, Handler);
}

void FActionSkillModule::RegisterEffectVariableHandler(const UStruct* Struct, FIndexEffectVariable Handler)
{
	if (!Struct) return;
	RegisteredEffectVariableHandler.Add(Struct, Handler);
}

FOnActionKeyEventNotified* FActionSkillModule::SearchActionNotifyHandler(const UStruct* Struct)
{
	if (RegisteredActionKeyNotifyHandler.Contains(Struct))
	{
		return &RegisteredActionKeyNotifyHandler[Struct];
	}

	return nullptr;
}

FIndexEffectVariable* FActionSkillModule::SearchEffectVariableHandler(const UStruct* Struct)
{
	if (RegisteredEffectVariableHandler.Contains(Struct))
	{
		return &RegisteredEffectVariableHandler[Struct];
	}

	return nullptr;
}

void FActionSkillModule::StartupModule()
{
	if (!SkillTable)
	{
		SkillTable = NewObject<UActionSkillTable>();
		SkillTable->AddToRoot();
	}

	if (!EventTable)
	{
		EventTable = NewObject<UActionEventTable>();
		EventTable->AddToRoot();
	}
}

void FActionSkillModule::ShutdownModule()
{
	if (SkillTable)
	{
		SkillTable->RemoveFromRoot();
	}

	if (EventTable)
	{
		EventTable->RemoveFromRoot();
	}
}

FActionSkillPtr FActionSkillModule::GetSkillById(int32 Id)
{
	if (SkillTable && SkillTable->ActionSkills.Contains(Id))
	{
		return SkillTable->ActionSkills[Id];
	}

	return nullptr;
}

FActionEventPtr FActionSkillModule::GetActionEventById(int32 Id)
{
	if (EventTable && EventTable->ActionEvents.Contains(Id))
	{
		return EventTable->ActionEvents[Id];
	}

	return nullptr;
}

bool FActionSkillModule::LoadSkillConfig()
{
	// 1. Load SkillTable
	if (SkillTable)
	{
		FString ConfigPath = GetDefault<UActionSkillSettings>()->SkillTablePath.FilePath;
		if (!ConfigPath.IsEmpty())
		{
			FString SkillTableString;
			if (FFileHelper::LoadFileToString(SkillTableString, *ConfigPath, FFileHelper::EHashOptions::None))
			{
				TArray<FString> Errors;
				FActionSkillTableCSVImporter Importer(SkillTable, SkillTableString, Errors);
				Importer.ReadTable();
				if (Errors.Num())
				{
					UE_LOG(LogActionSkill, Warning, TEXT(">>>>>>>>>>>>>>>> Load Skill Table Errors <<<<<<<<<<<<<"));
					for (const FString& Error : Errors)
					{
						UE_LOG(LogActionSkill, Warning, TEXT("%s"), *Error);
					}
				}
			}
		}
	}

	// 2. Load EventTable
	if (EventTable)
	{
		FString ConfigPath = GetDefault<UActionSkillSettings>()->EventTablePath.FilePath;
		if (!ConfigPath.IsEmpty())
		{
			FString EventTableString;
			if (FFileHelper::LoadFileToString(EventTableString, *ConfigPath))
			{
				TArray<FString> Errors;
				FActionEventTableCSVImporter Importer(EventTable, EventTableString, Errors);
				Importer.ReadTable();
				if (Errors.Num())
				{
					UE_LOG(LogActionSkill, Warning, TEXT(">>>>>>>>>>>>>>>> Load Event Table Errors <<<<<<<<<<<<<"));
					for (const FString& Error : Errors)
					{
						UE_LOG(LogActionSkill, Warning, TEXT("%s"), *Error);
					}
				}
			}
		}
	}

	return true;
}

#if WITH_EDITOR
bool FActionSkillModule::ExportSkillConfigs(FString& OutSkills, FString& OutEvents)
{
	bool bRet = SkillTable && EventTable;
	if (SkillTable)
	{
		FActionSkillTableCSVExporter Exporter(SkillTable, OutSkills);
		Exporter.WriteTable();
	}

	if (EventTable)
	{
		FActionEventTableCSVExporter Exporter(EventTable, OutEvents);
		Exporter.WriteTable();
	}

	return bRet;
}

void FActionSkillModule::PostOnSaveData()
{
	if (SkillTable) SkillTable->OnSaveData();
	if (EventTable) EventTable->OnSaveData();
}
#endif

FActionSkillModule& FActionSkillModule::Get()
{
	FActionSkillModule& ASModule = FModuleManager::Get().LoadModuleChecked<FActionSkillModule>("ActionSkill");
	return ASModule;
}
IMPLEMENT_MODULE(FActionSkillModule, ActionSkill);
#undef LOCTEXT_NAMESPACE
	
