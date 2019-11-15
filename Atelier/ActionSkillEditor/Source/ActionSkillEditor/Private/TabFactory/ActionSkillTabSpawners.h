// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-21

#pragma once

#include "CoreMinimal.h"

class FWorkflowCentricApplication;
class FWorkflowAllowedTabSet;

namespace TabSpawners
{
	extern void RegisterActionSkillTabs(const TSharedRef<FWorkflowCentricApplication> HostingAppPt, FWorkflowAllowedTabSet& TabFactories);
};
