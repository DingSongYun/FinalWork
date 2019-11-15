#pragma once

#include "System/Project.h"

class FEditorUtilities
{
public:
	static void OpenLevelByName(const FString& LevelName);

	static FString OpenPickFileDialog(const FString& RootDir, const FString& DialogTitle, const FString& Types);

	static FString OpenSaveFileDialog(const FString& RootDir, const FString& DialogTitle, const FString& Types);

	static bool DumpRowToDataTable(class UDataTable* ToDataTable, FName RowName, const void* RowData);

	static void CleanCurrentLevel(bool CleanLoadedMap, bool CleanConfigObjs);
};
