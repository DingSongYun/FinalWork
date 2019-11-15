// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-12

#pragma once

#include "CoreMinimal.h"
#include "LuaState.h"
#include "SkillEditorProxy.h"

class FSkillEditorMiddleware
{
public:
    static void OnCreateEditor();
    static void OnExitEditor();
    static void OnInitializeEditor();
    static void PostCreateEditor(TSharedPtr<class FActionSkillEditor> InSkillEditorPtr);
    static TSharedPtr<class FActionSkillEditor> GetSkillEditor();
    static USkillEditorProxy* GetSkillEditorProxy(class UWorld* InWorld = nullptr);
    FORCEINLINE static ISkillPreviewProxy* GetSkillPreviewProxy(class UWorld* InWorld) { return static_cast<ISkillPreviewProxy*>(GetSkillEditorProxy(InWorld)); }
private:
    static USkillEditorProxy* LuaEditorPtr;
    static TSharedPtr<class FActionSkillEditor> SkillEditorPtr;
};