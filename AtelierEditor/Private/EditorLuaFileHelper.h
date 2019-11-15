#pragma once
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SCompoundWidget.h"
#include "AssetData.h"
#include "Widgets/Views/STableRow.h"
#include "System/Project.h"

class FEditorLuaFileHelper
{
public:
	static FString ReadValueFromLuaTable(const FString& tKey, const TCHAR* tFileName);
	static void SaveValueToLuaTable(const FString& tKey, const FString& tValue, const TCHAR* tFileName);

private:
	const FString rootPath = FPaths::ProjectContentDir();
};
