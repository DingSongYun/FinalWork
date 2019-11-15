// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-08-26

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class USkeleton;

class SSkillAssetBrowserTab : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SSkillAssetBrowserTab)
    {}

    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs, TWeakPtr<class IActionSkillEditor> InEditorPtr);

private:
    /** Filter skill asset */
    bool HandleFilterAsset(const FAssetData& InAssetData) const;
    /** Get preview character skeleton */
    USkeleton* GetSkillCharacterSkeleton() const;
    /** Open Asset */
    void OnRequestOpenAsset(const FAssetData& AssetData);

private:
    /** Pointer to ActionSkillEditor. */
    TWeakPtr<class FActionSkillEditor> ActionSkillEditor;
};

