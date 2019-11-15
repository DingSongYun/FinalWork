#include "StageTableHelper.h"
#include "Serialization/Csv/CsvParser.h"
#include "StageEditor.h"
#include "Paths.h"
#include "FileHelper.h"
#include "Logger.h"
#include "StageTable.h"

/*********************************************************************/
// FCSVImporter
/*********************************************************************/
void FCSVImporter::ReadCSV(const FString& InCSVData, FSerializeResult& OutProblem)
{
    if (InCSVData.IsEmpty())
    {
        OutProblem.AddProblem(TEXT("CSV data is empty."));
        return;
    }

    const FCsvParser Parser(InCSVData);
    const auto& Rows = Parser.GetRows();


    // Parse Head Line => Column
    TMap<FName, int32> HeadNames;
    const int32 HeadRowIndex = GetSkipRowNum();
    const TArray<const TCHAR*>& HeadRow = Rows[HeadRowIndex];
    for (int32 Index = 0; Index < HeadRow.Num(); Index ++)
    {
        HeadNames.Add(DataTableUtils::MakeValidName(HeadRow[Index]), Index);
    }

    // Handler data rows
    for (int32 RowIndex = 1 + HeadRowIndex; RowIndex < Rows.Num(); RowIndex ++)
    {
        const TArray<const TCHAR*>& Cells = Rows[RowIndex];
        if (Cells.Num() < 1)
        {
            OutProblem.AddProblem(FString::Printf(TEXT("Row %d has too few cells."), RowIndex));
            continue;
        }
        ImportRow(HeadNames, Cells, OutProblem);
    }
}

/*********************************************************************/
// FStageTableImporter
/*********************************************************************/
FStageTableImporter::FStageTableImporter(FStageTable* InStageTable,
    const FString& InCSVData,
    FSerializeResult& OutProblem)
    : FCSVImporter(InCSVData, OutProblem), StageTable(InStageTable)
{
    check(StageTable);
}

void FStageTableImporter::PreImport()
{
    StageTable->Empty();
}

void FStageTableImporter::ImportRow(const TMap<FName, int32>& Headers,
    const TArray<const TCHAR*>& RowCells,
    FSerializeResult& OutProblem)
{
    FStageScopePtr StageScope = StageTable->ConstructStage();
    StructSerializer::ImportCSVRow( Headers, RowCells,
                                    Cast<UScriptStruct>(StageScope->GetStruct()),
                                    StageScope->GetStructMemory(),
                                    SerializeUtils::UseLuaString,
                                    OutProblem );
    StageTable->AddStage(StageScope);
}

void FStageTableImporter::PostImport()
{
}

void FStageTabeleExporter::ExportTable( const FStageTable* StageTable,
    FString& OutCSVData,
    FSerializeResult& OutProblem)
{
    OutCSVData.Empty();
    UScriptStruct& Struct = StageTable->GetStageStruct();
    for (TFieldIterator<UProperty> It(&Struct); It; ++It)
    {
        UProperty* Property = *It;
        check(Property );

        OutCSVData += SerializeUtils::GetPropertyExportName(Property);
        OutCSVData += TEXT(",");
    }
    OutCSVData += TEXT("\n");

    for (const FStageScopePtr& StageScope : StageTable->GetStageList())
    {
        StructSerializer::ExportCSVRow(OutCSVData, &Struct, StageScope->GetStructMemory(), SerializeUtils::UseLuaString, OutProblem);

        OutCSVData += TEXT("\n");
    }
}

bool FStageTableHelper::ImportStageTable(FStageTable* StagTable, const FString& InCSVData)
{
    FSerializeResult OutProblem;
    FStageTableImporter Importer(StagTable, InCSVData, OutProblem);
    Importer.DoImport();
    if (OutProblem.HasError())
    {
        UE_LOG(LogStageEditor, Error, TEXT("%s"), *OutProblem.ToString());
    }
    return true;
}

bool FStageTableHelper::ExportStageTable(const FStageTable* StageTable, FString& OutCSVData)
{
    FSerializeResult OutProblem;
    FStageTabeleExporter Importer(StageTable, OutCSVData, OutProblem);
    if (OutProblem.HasError())
    {
        UE_LOG(LogStageEditor, Error, TEXT("%s"), *OutProblem.ToString());
    }
    return true;
}

bool FStageTableHelper::ImportStageDataFile(FStageTable* StageTable, const FString& FilePath)
{
    FString FileString;
    if (FFileHelper::LoadFileToString(FileString, *FilePath))
    {
        ImportStageTable(StageTable, FileString);
    }
    return true;
}

bool FStageTableHelper::ExportStageDataFile(const FStageTable* StageTable, const FString& FilePath)
{
    FString OutString;
    ExportStageTable(StageTable, OutString);
    if (!FFileHelper::SaveStringToFile(OutString, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), EFileWrite::FILEWRITE_NoFail))
    {
        UE_LOG(LogStageEditor, Error, TEXT("Error to save skill data on path: %s"), *FilePath);
    }

    return true;
}
