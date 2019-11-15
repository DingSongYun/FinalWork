// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-26

#include "ActionSkillTableCSV.h"
#include "ActionSkill.h"
#include "Serialization/Csv/CsvParser.h"
#include "Serialization/SerializeUtils.h"
#include "DataTableUtils.h"
#include "UObject/StructOnScope.h"

const int32 ROW_NUM_SKIP = 0;
const int32 ID_INVALID = -1;

FString GetPropertyShowName(const UProperty* InProp)
{
	if (InProp)
	{
		return DataTableUtils::GetPropertyExportName(InProp, EDataTableExportFlags::UsePrettyPropertyNames | EDataTableExportFlags::UsePrettyEnumNames);
	}

	return FString();
}

/*********************************************************************/
// Action SkillTable Importer & Exporter
/*********************************************************************/
#if WITH_EDITOR
FActionSkillTableCSVExporter::FActionSkillTableCSVExporter(UActionSkillTable* InSkillTable, FString& OutExportText)
	: SkillTable(InSkillTable), ExportedText(OutExportText)
{}

bool FActionSkillTableCSVExporter::WriteTable()
{
	UScriptStruct& SkillStruct = SkillTable->GetSkillStruct();

	// Write the header (column titles)
	ExportedText.Empty();
	for (TFieldIterator<UProperty> It(&SkillStruct); It; ++It)
	{
		UProperty* Property = *It;
		check(Property );

		ExportedText += GetPropertyShowName(Property);
		ExportedText += TEXT(",");
	}
	ExportedText += TEXT("\n");

	for (TPair<int32, FActionSkillPtr>& Pair: SkillTable->ActionSkills)
	{
		FActionSkillPtr Skill = Pair.Value;

		// write row
		for (TFieldIterator<UProperty> It(&SkillStruct); It; ++It)
		{
			UProperty* BaseProp = *It;
			check(BaseProp);

			const uint8* Data = (uint8*)BaseProp->ContainerPtrToValuePtr<void>(Skill->GetData()->GetStructMemory(), 0);
			const FString PropertyValue = SerializeUtils::GetPropertyValueAsString(BaseProp, Data, SerializeUtils::UseLuaString);
			//const FString PropertyValue = SerializeUtils::GetPropertyValueAsString(BaseProp, Skill->GetData()->GetStructMemory());
				// EDataTableExportFlags::UsePrettyPropertyNames | EDataTableExportFlags::UsePrettyEnumNames);
			ExportedText += TEXT("\"");
			// ExportedText += PropertyValue.Replace(TEXT("\""), TEXT("\"\""));
			ExportedText += PropertyValue.Replace(TEXT("\""), TEXT("'"));
			ExportedText += TEXT("\"");
			ExportedText += TEXT(",");
		}

		ExportedText += TEXT("\n");
	}
	return true;
}
#endif // WITH_EDITOR

FActionSkillTableCSVImporter::FActionSkillTableCSVImporter(UActionSkillTable* InSkillTable, FString& InCSVData, TArray<FString>& OutProblems)
	: SkillTable(InSkillTable), CSVData(InCSVData), ImportProblems(OutProblems)
{}

bool FActionSkillTableCSVImporter::ReadTable()
{
	if (CSVData.IsEmpty())
	{
		ImportProblems.Add(TEXT("CSV data is empty."));
		return false;
	}

	SkillTable->EmptyTable();

	const FCsvParser Parser(CSVData);
	const auto& Rows = Parser.GetRows();

	// Parse Head Line
	TMap<FName, int32> HeadNames;
	const TArray<const TCHAR*>& HeadRow = Rows[ROW_NUM_SKIP];
	for (int32 Index = 0; Index < HeadRow.Num(); Index ++)
	{
		HeadNames.Add(DataTableUtils::MakeValidName(HeadRow[Index]), Index);
	}

	for (int32 RowIndex = 1 + ROW_NUM_SKIP; RowIndex < Rows.Num(); RowIndex++)
	{
		const TArray<const TCHAR*>& Cells = Rows[RowIndex];

		if (Cells.Num() < 1)
		{
			ImportProblems.Add(FString::Printf(TEXT("Row %d has too few cells."), RowIndex));
			continue;
		}

		UScriptStruct& ActionStruct = SkillTable->GetSkillStruct();
#if false
		uint8* RowData = (uint8*)FMemory::Malloc(SkillTable->GetSkillStruct()->GetStructureSize());
		ActionStruct->InitializeStruct(RowData);
#else
		FActionSkillPtr RowData = MakeShareable(new FActionSkillScope());
		RowData->Initialize(UActionSkillTable::GetSkillStruct());
#if WITH_EDITOR
		RowData->OnSkillIdChanged.BindUObject(SkillTable, &UActionSkillTable::OnSkillIdChanged);
		RowData->OnSelectedEventIdChanged.BindUObject(SkillTable, &UActionSkillTable::OnSelectedEventIdChanged);
#endif
#endif

		for (TFieldIterator<UProperty> PropIt(&ActionStruct); PropIt; ++PropIt)
		{
			UProperty* Property = *PropIt;
			//FName PropertyName = FName(*Property->GetShowName());
			FName PropertyName = FName(*GetPropertyShowName(Property));

			if (!HeadNames.Contains(PropertyName))
			{
				ImportProblems.Add(FString::Printf(TEXT("Skill has more properies(%s) than csv row"), *PropertyName.ToString()));
				continue;
			}

			int32 ColumnIndex = HeadNames.FindRef(PropertyName);
			FString PropertyString = Cells[ColumnIndex];
			PropertyString = PropertyString.Replace(TEXT("'"), TEXT("\""));
			uint8* Data = (uint8*)Property->ContainerPtrToValuePtr<void>(RowData->GetData()->GetStructMemory(), 0);
			FString Error = SerializeUtils::AssignStringToProperty(PropertyString, Property, Data, SerializeUtils::UseLuaString);
			if(Error.Len() > 0)
			{
				FString ColumnName = (Property != NULL) 
					? DataTableUtils::GetPropertyDisplayName(Property, Property->GetName())
					: FString(TEXT("NONE"));
				ImportProblems.Add(FString::Printf(TEXT("Problem assigning string '%s' to property '%s' : %s"), Cells[ColumnIndex], *ColumnName, *Error));
			}
		}

		int32 RowId = RowData->GetId();
		if (RowId == ID_INVALID)
		{
			ImportProblems.Add(FString::Printf(TEXT("Row %d has an invalid id."), RowIndex));
			continue;
		}

		if (SkillTable->ActionSkills.Find(RowId))
		{
			ImportProblems.Add(FString::Printf(TEXT("Duplicate skill id: %d."), RowId));
			continue;
		}

		SkillTable->ActionSkills.Add(RowId, RowData);
#if WITH_EDITOR
		SkillTable->UpdateNextValideSkillId(RowId);
		RowData->MarkDirty(false);
#endif
	}

	SkillTable->Modify(true);
	return true;
}

/*********************************************************************/
// Action EventTable Importer & Exporter
/*********************************************************************/
FActionEventTableCSVImporter::FActionEventTableCSVImporter(UActionEventTable* InEventTable, FString& InCSVData, TArray<FString>& OutProblems)
	: EventTable(InEventTable), CSVData(InCSVData), ImportProblems(OutProblems)
{}

bool FActionEventTableCSVImporter::ReadTable()
{
	if (CSVData.IsEmpty())
	{
		ImportProblems.Add(TEXT("CSV data is empty."));
		return false;
	}

	EventTable->EmptyTable();

	const FCsvParser Parser(CSVData);
	const auto& Rows = Parser.GetRows();

	// Parse Head Line
	TMap<FName, int32> HeadNames;
	const TArray<const TCHAR*>& HeadRow = Rows[ROW_NUM_SKIP];
	for (int32 Index = 0; Index < HeadRow.Num(); Index ++)
	{
		HeadNames.Add(DataTableUtils::MakeValidName(HeadRow[Index]), Index);
	}
	static UScriptStruct* EventStruct = FActionEvent::StaticStruct();
	for (int32 RowIndex = 1 + ROW_NUM_SKIP; RowIndex < Rows.Num(); RowIndex++)
	{
		const TArray<const TCHAR*>& Cells = Rows[RowIndex];

		if (Cells.Num() < 1)
		{
			ImportProblems.Add(FString::Printf(TEXT("Row %d has too few cells."), RowIndex));
			continue;
		}

		FActionEventPtr NewEvent = MakeShareable(new FActionEvent());
#if WITH_EDITOR
		NewEvent->OnEventIdChanged.BindUObject(EventTable, &UActionEventTable::OnEventIdChanged);
#endif
		for (TFieldIterator<UProperty> PropIt(EventStruct); PropIt; ++PropIt)
		{
			UProperty* Property = *PropIt;
			//FName PropertyName = FName(*Property->GetShowName());
			FName PropertyName = FName(*GetPropertyShowName(Property));

			if (!HeadNames.Contains(PropertyName))
			{
				ImportProblems.Add(FString::Printf(TEXT("Event has more properies(%s) than csv row"), *PropertyName.ToString()));
				continue;
			}

			int32 ColumnIndex = HeadNames.FindRef(PropertyName);
			FString PropertyString = Cells[ColumnIndex];
			PropertyString = PropertyString.Replace(TEXT("'"), TEXT("\""));
			uint8* Data = (uint8*)Property->ContainerPtrToValuePtr<void>((void*)(NewEvent.Get()), 0);
			FString Error = SerializeUtils::AssignStringToProperty(PropertyString, Property, Data);
			if(Error.Len() > 0)
			{
				FString ColumnName = (Property != NULL) 
					? DataTableUtils::GetPropertyDisplayName(Property, Property->GetName())
					: FString(TEXT("NONE"));
				ImportProblems.Add(FString::Printf(TEXT("Problem assigning string '%s' to property '%s' : %s"), Cells[ColumnIndex], *ColumnName, *Error));
			}
		}

		int32 RowId = NewEvent->Id;
		if (RowId == ID_INVALID)
		{
			ImportProblems.Add(FString::Printf(TEXT("Row %d has an invalid id."), RowIndex));
			continue;
		}

		if (EventTable->ActionEvents.Find(RowId))
		{
			ImportProblems.Add(FString::Printf(TEXT("Duplicate event id: %d."), RowId));
			continue;
		}

		EventTable->ActionEvents.Add(RowId, NewEvent);
#if WITH_EDITOR
		EventTable->UpdateNextValideId(RowId);
		NewEvent->MarkDirty(false);
		NewEvent->Params.FixedUserStructType();
#endif
	}
	return true;
}

#if WITH_EDITOR
FActionEventTableCSVExporter::FActionEventTableCSVExporter(UActionEventTable* InEventTable, FString& OutExportText)
	: EventTable(InEventTable), ExportedText(OutExportText)
{}

bool FActionEventTableCSVExporter::WriteTable()
{
	// Write the header (column titles)
	ExportedText.Empty();
	static UScriptStruct* EventStruct = FActionEvent::StaticStruct();
	for (TFieldIterator<UProperty> It(EventStruct); It; ++It)
	{
		UProperty* Property = *It;
		check(Property );

		ExportedText += GetPropertyShowName(Property);
		ExportedText += TEXT(",");
	}
	ExportedText += TEXT("\n");

	for (TPair<int32, FActionEventPtr>& Pair: EventTable->ActionEvents)
	{
		FActionEventPtr Event = Pair.Value;
		// Event->ExportInnerUserStruct();
		// write row
		for (TFieldIterator<UProperty> It(EventStruct); It; ++It)
		{
			UProperty* BaseProp = *It;
			check(BaseProp);

			const uint8* Data = (uint8*)BaseProp->ContainerPtrToValuePtr<void>(Event.Get(), 0);
			const FString PropertyValue = SerializeUtils::GetPropertyValueAsString(BaseProp, Data);
			ExportedText += TEXT("\"");
			ExportedText += PropertyValue.Replace(TEXT("\""), TEXT("'"));
			ExportedText += TEXT("\"");
			ExportedText += TEXT(",");
		}

		ExportedText += TEXT("\n");
	}
	return true;
}
#endif // WITH_EDITOR