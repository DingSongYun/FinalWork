// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-26

#pragma once

#include "CoreMinimal.h"

class UActionSkillTable;
class UActionEventTable;

/*********************************************************************/
// Action SkillTable Importer & Exporter
/*********************************************************************/
#if WITH_EDITOR
class ACTIONSKILL_API FActionSkillTableCSVExporter
{
public:
	FActionSkillTableCSVExporter(UActionSkillTable* InSkillTable, FString& OutExportText);
	virtual ~FActionSkillTableCSVExporter() {}

	bool WriteTable();
private:
	UActionSkillTable* SkillTable;
	FString& ExportedText;
};
#endif // WITH_EDITOR

class ACTIONSKILL_API FActionSkillTableCSVImporter
{
public:
	FActionSkillTableCSVImporter(UActionSkillTable* InSkillTable, FString& InCSVData, TArray<FString>& OutProblems);
	virtual ~FActionSkillTableCSVImporter() {}

	bool ReadTable();

private:
	UActionSkillTable* SkillTable;
	FString CSVData;
	TArray<FString>& ImportProblems;
};

/*********************************************************************/
// Action EventTable Importer & Exporter
/*********************************************************************/
class ACTIONSKILL_API FActionEventTableCSVImporter
{
public:
	FActionEventTableCSVImporter(UActionEventTable* InEventTable, FString& InCSVData, TArray<FString>& OutProblems);
	virtual ~FActionEventTableCSVImporter() {}

	bool ReadTable();

private:
	UActionEventTable* EventTable;
	FString CSVData;
	TArray<FString>& ImportProblems;
};

#if WITH_EDITOR
class ACTIONSKILL_API FActionEventTableCSVExporter
{
public:
	FActionEventTableCSVExporter(UActionEventTable* InEventTable, FString& OutExportText);
	virtual ~FActionEventTableCSVExporter() {}

	bool WriteTable();
private:
	UActionEventTable* EventTable;
	FString& ExportedText;
};
#endif // WITH_EDITOR