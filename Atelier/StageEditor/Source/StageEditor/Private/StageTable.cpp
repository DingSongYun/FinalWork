#include "StageTable.h"

void FStageTable::Empty()
{
	StageList.Empty();
}

UScriptStruct & FStageTable::GetStageStruct() const
{
	return FStageEditorModule::GetStageStruct();
}

FStageScopePtr FStageTable::ConstructStage()
{
	FStageScopePtr StageScope = MakeShareable(new FUserStructOnScope());
	StageScope->Initialize(GetStageStruct());
	return StageScope;
}

void FStageTable::AddStage(FStageScopePtr InStage, int32 NewAtIndex)
{
	if (InStage.IsValid())
	{
		if (NewAtIndex >= 0)
		{
			StageList.Insert(InStage, NewAtIndex);
		}
		else
		{
			StageList.Add(InStage);
		}
	}
}

FStageScopePtr FStageTable::NewAndAddStage(int32 NewAtIndex)
{
	FStageScopePtr NewStage = ConstructStage();
	AddStage(NewStage, NewAtIndex);
	return NewStage;
}

void FStageTable::DeleteStage(FStageScopePtr Stage)
{
	int index = 0;
	for (const FStageScopePtr& StageScope : StageList)
	{
		if (StageScope == Stage)
		{
			StageList.RemoveAt(index);
			break;
		}
		index++;
	}
}
