// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-29
#pragma once

#include "IStageEditorInterface.h"
#include "Lua/ToLuaInterface.h"
#include "Tickable.h"
#include "StageEditorProxy.generated.h"

class UWorld;

UCLASS()
class ULuaStageEdProxy : public UObject, public IToLuaInterface
{
    GENERATED_BODY()
    GENERATED_LUAINTERFACE_BODY()
public:
    void AssembleCharacter(class ACharacter* InCharacter, int32 NpcId);
    void InitializeEditor();
    void SetWeather(uint32 WeatherId, const FName& WeatherName);
    void OnStageMapChanged();
};

class FStageEditorProxy : public IStageEditorInterface, public FGCObject
{
public:
    FStageEditorProxy();
    virtual ~FStageEditorProxy();
    virtual void StartupEditor() override;
    virtual void ShutdownEditor() override;
    virtual void InitEditorContext() override;
    virtual void ExitEditorContext() override;
    virtual void OnStageMapChanged() override;
    virtual void CustomStageDetailLayout(class IDetailsView* DetailsView) override;
    virtual TSharedRef<class SWidget> BuildExternalPanel() override;

    void OnExitEditorContext();
    virtual void SetWeather(uint32 WeatherId, const FName& WeatherName);
    void AddReferencedObjects(FReferenceCollector& Collector) override;

   static ULuaStageEdProxy* GetLuaEdProxy() { return sLuaEditorPtr; }
   UWorld* GetWorld();
private:
    // void OnMapChanged( UWorld* World, EMapChangeType MapChangeType );

private:
    bool bInitialized;
    static ULuaStageEdProxy* sLuaEditorPtr;
};