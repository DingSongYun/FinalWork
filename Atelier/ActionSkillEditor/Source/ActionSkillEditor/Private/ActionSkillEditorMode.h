// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-19
#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

class FActionSkillEditorMode : public FApplicationMode
{
public:
	FActionSkillEditorMode(TSharedRef<class FWorkflowCentricApplication> InHostingApp);
	void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;

	void PreDeactivateMode() override;
	void PostActivateMode() override {}
protected:
	void AddTabFactory(FCreateWorkflowTabFactory FactoryCreator) override;
	void RemoveTabFactory(FName TabFactoryID) override;

protected:
	TWeakPtr<class FWorkflowCentricApplication> HostingAppPtr;

	/** The tab factories we support */
	FWorkflowAllowedTabSet TabFactories;
};
