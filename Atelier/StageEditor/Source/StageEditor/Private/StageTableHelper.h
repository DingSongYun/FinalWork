// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-16-12

#pragma once
#include "Serialization/SerializeUtils.h"
#include "StageTable.h"

class FCSVImporter
{
public:
    FCSVImporter(const FString& InCSVData, FSerializeResult& OutProblem)
        : CSVData(InCSVData), ImportProblem(OutProblem)
    {}

    virtual ~FCSVImporter() {}

    void DoImport()
    {
        PreImport();
        ReadCSV(CSVData, ImportProblem);
        PostImport();
    }


    virtual int32 GetSkipRowNum() { return 0; }
    virtual void PreImport() {}
    virtual void ImportRow( const TMap<FName, int32>& Headers,
                            const TArray<const TCHAR*>& RowCells,
                            FSerializeResult& OutProblem ) {}
    virtual void PostImport() {}

private:
    void ReadCSV(const FString& InCSVData, FSerializeResult& OutProblem);

protected:
    const FString& CSVData;
    FSerializeResult& ImportProblem;
};

class FStageTableImporter : public FCSVImporter
{
public:
    FStageTableImporter( FStageTable* StageTable,
                        const FString& InCSVData,
                        FSerializeResult& OutProblem);

    virtual void PreImport() override;
    virtual void ImportRow( const TMap<FName, int32>& Headers,
                            const TArray<const TCHAR*>& RowCells,
                            FSerializeResult& OutProblem ) override;
    virtual void PostImport() override;

private:
    FStageTable* StageTable;
};

class FStageTabeleExporter
{
public:
    FStageTabeleExporter(const FStageTable* StageTable,
        FString& InCSVData,
        FSerializeResult& OutProblem)
    {
        ExportTable(StageTable, InCSVData, OutProblem);
    }

    void ExportTable( const FStageTable* StageTable,
                        FString& InCSVData,
                        FSerializeResult& OutProblem);
};

class FStageTableHelper
{
public:
    static bool ImportStageTable(FStageTable* StageTable, const FString& InString);
    static bool ExportStageTable(const FStageTable* StageTable, FString& OutString);
    static bool ImportStageDataFile(FStageTable* StageTable, const FString& FilePath);
    static bool ExportStageDataFile(const FStageTable* StageTable, const FString& FilePath);
};