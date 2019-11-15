// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-29

#pragma once

class IStageEditorInterface
{
public:
    virtual void StartupEditor() = 0;
    virtual void ShutdownEditor() = 0;
    virtual void InitEditorContext() = 0;
    virtual void ExitEditorContext() = 0;
    virtual void OnStageMapChanged() = 0;
    virtual void CustomStageDetailLayout(class IDetailsView* DetailsView) = 0;
    virtual TSharedRef<class SWidget> BuildExternalPanel() = 0;
};