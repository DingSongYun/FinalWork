// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-16

#pragma once
#include "CoreMinimal.h"

class EDCOMMONLIBS_API FEditorUtilities
{
public:
    /** 打开地图 */
    static void OpenLevelByName(const FString& LevelName);

    /** 选择文件对话框 */
    static FString OpenPickFileDialog(const FString& RootDir, const FString& DialogTitle, const FString& Types);

    /** 保存文件对话框 */
    static FString OpenSaveFileDialog(const FString& RootDir, const FString& DialogTitle, const FString& Types);

    static void PlayInEditor();
    static void StopEditorPlaySession();
};