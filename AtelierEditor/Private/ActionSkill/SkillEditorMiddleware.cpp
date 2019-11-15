// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-12

#include "SkillEditorMiddleware.h"
#include "Kismet/GABlueprintFunctionLibrary.h"
#include "SkillEditorProxy.h"
#include "Engine/World.h"
#include "Editor/EditorEngine.h"
#include "EditorStaticLibrary.h"
#include "Lua/LuaExtendFunctionLibrary.h"
#include "ActionSkillEditor.h"
#include "ActionSkillEditorPreviewScene.h"
#include "Editor.h"

// #include "CommonCode/General/Lua/SLuaManager.h"
USkillEditorProxy* FSkillEditorMiddleware::LuaEditorPtr = nullptr;
TSharedPtr<FActionSkillEditor> FSkillEditorMiddleware::SkillEditorPtr = nullptr;

void FSkillEditorMiddleware::OnCreateEditor()
{
    // 1. start lua editor
    // LuaManager::GetInstance()->StartEdLua();
    UGABlueprintFunctionLibrary::StartEditorLua();
}

void FSkillEditorMiddleware::PostCreateEditor(TSharedPtr<class FActionSkillEditor> InSkillEditorPtr)
{
    SkillEditorPtr = InSkillEditorPtr;
    FSkillEditorMiddleware::GetSkillEditorProxy()->Initialize();
}

void FSkillEditorMiddleware::OnInitializeEditor()
{
}

void FSkillEditorMiddleware::OnExitEditor()
{
    // 1. shutdown lua editor
    // LuaManager::GetInstance()->StopEdLua();
    UGABlueprintFunctionLibrary::StopEditorLua();
    if (LuaEditorPtr)
    {
        LuaEditorPtr->RemoveFromRoot();
        LuaEditorPtr = nullptr;
    }
    SkillEditorPtr = nullptr;
}

USkillEditorProxy* FSkillEditorMiddleware::GetSkillEditorProxy(UWorld* InWorld)
{
    if (LuaEditorPtr == nullptr)
    {
        if (InWorld == nullptr)
        {
            InWorld = GEditor->GetEditorWorldContext().World();
        }
        LuaEditorPtr = NewObject<USkillEditorProxy>(InWorld);
        LuaEditorPtr->SetLuaBPVar_Call(
            UEditorStaticLibrary::CallToLua("getSkillEditor")
        );
        LuaEditorPtr->AddToRoot();
    }
    return LuaEditorPtr;
}

TSharedPtr<class FActionSkillEditor> FSkillEditorMiddleware::GetSkillEditor()
{
    return SkillEditorPtr;
}