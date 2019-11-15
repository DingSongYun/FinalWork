// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-11-13

#pragma once
#include "StageTable.h"

class IStageScene
{
public:
    /** 加载Stage到当前场景 */
    virtual void LoadStage(FStageScopePtr NewStage) = 0;
    /** 重刷当前场景的Stage */
    virtual void ReLoadStage() = 0;
    /** 清除当前场景中的Stage */
    virtual void CleanStage(bool bFullClean = false) = 0;
    /** 导出当前场景中Stage的Object Config File */
    virtual void ExportStageObjConfigs() = 0;
    /** 导出当前场景中Stage的Object Config File 到指定目录 */
    virtual void ExportStageObjConfigsAs(const FString& FilePath) = 0;
    /** 获取当前场景中的Stage对象，可能为空 */
    virtual FStageScopePtr GetCurrentStage() = 0;
    /** Stage对象属性改变时的 Notify Hook*/
    virtual class FNotifyHook* GetStageSettingChangedHook() = 0;

    /** 获取IStageScene单例 */
    static IStageScene* Get();
};