// Author: SongYun.Ding
// Date: 2019-10-29
#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"

class ULevelStreamingDynamic;
class UWorld;
class SWindow;
class UProgressBar;

class FLevelAsyncLoader : public FTickableGameObject
{
    bool bLoadSubLevels;
    ULevelStreamingDynamic* LoadingLevelStreaming;
    UWorld* World;
    FSimpleDelegate OnLoadedDelegate;
    // TSharedPtr< SWindow > ProgressWindow;
    UProgressBar* ProgressWindow;
public:
    FLevelAsyncLoader ();
    ~FLevelAsyncLoader ();
    ULevelStreamingDynamic* AsyncLoadLevel(UWorld* World, 
                                        const FString& LevelPackageName, 
                                        FSimpleDelegate OnLoadedDelegate, 
                                        bool bWithSubLevels = true, 
                                        bool bShowProgressDialog = true);

    ULevelStreamingDynamic* AsyncLoadLevel(UWorld* World,
                                        const TSoftObjectPtr<UWorld> InLevel,
                                        FSimpleDelegate OnLoadedDelegate,
                                        bool bWithSubLevels = true,
                                        bool bShowProgressDialog = true);
    void OnLoadCompleted();

    virtual bool IsTickableInEditor() const override
    {
        return true;
    }

    virtual bool IsTickable() const override
    {
        return true;
    }

    virtual void Tick( float DeltaTime ) override;

    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT( FLevelAsyncLoader, STATGROUP_Tickables );
    }

    virtual ETickableTickType GetTickableTickType() const override
    {
        return ETickableTickType::Always;
    }

    virtual bool IsTickableWhenPaused() const
    {
        return true;
    }

private:
    void ProcessSubLevels();
    void MakeProgressDialog();
    void ShowProgressDialog();
    void HideProgressDialog();
};