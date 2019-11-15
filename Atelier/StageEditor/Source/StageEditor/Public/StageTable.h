#pragma once

#include "StageEditor.h"
#include "UserStructOnScope.h"

typedef TSharedPtr<FUserStructOnScope> FStageScopePtr;

class FStageTable
{
public:
	void Empty();
	UScriptStruct& GetStageStruct() const;
	FStageScopePtr ConstructStage();
	TArray<FStageScopePtr> GetStageList() const { return StageList; }
	FStageScopePtr NewAndAddStage(int32 NewAtIndex = INDEX_NONE);
	void AddStage(FStageScopePtr InStage, int32 NewAtIndex = INDEX_NONE);
	void DeleteStage(FStageScopePtr Stage);

private:
	TArray<FStageScopePtr> StageList;
};