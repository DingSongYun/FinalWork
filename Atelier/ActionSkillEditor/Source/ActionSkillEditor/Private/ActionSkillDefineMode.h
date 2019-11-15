// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-05
#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

class FActionSkillDefineMode : public FApplicationMode
{
public:
	FActionSkillDefineMode(TSharedRef<class FWorkflowCentricApplication> InHostingApp);
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	TArray<UObject*> GetObjectToSave() const { return ObjectsToSave; }
	void AddObjectToSave(UObject* InObject);

protected:
	virtual void AddTabFactory(FCreateWorkflowTabFactory FactoryCreator) override;
	virtual void RemoveTabFactory(FName TabFactoryID) override;
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager);

	TSharedRef<SDockTab> SpawnSkillStructureTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnEventStructureTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnSubEventStructureTab(const FSpawnTabArgs& Args);

private:
	static TSharedRef<SDockTab> SpawnSingleStructureTab(const FSpawnTabArgs& Args, UScriptStruct* Struct, FText TabLabel);

protected:
	TWeakPtr<class FWorkflowCentricApplication> HostingAppPtr;

	/** The tab factories we support */
	FWorkflowAllowedTabSet TabFactories;

	/** Struct To Save */
	TArray<UObject*> ObjectsToSave;
};