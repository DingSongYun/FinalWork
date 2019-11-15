#include "LevelAsyncLoader.h"
#include "Widget.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"
#include "CoreGlobals.h"
#include "ModuleManager.h" 
#include "Widgets/SWindow.h"
#include "Editor.h"
#include "UserWidget.h"
#include "Components/ProgressBar.h"

#define LOCTEXT_NAMESPACE "LevelAsyncLoader"

FLevelAsyncLoader::FLevelAsyncLoader ()
{
}

FLevelAsyncLoader::~FLevelAsyncLoader ()
{
}

ULevelStreamingDynamic* FLevelAsyncLoader::AsyncLoadLevel(UWorld* InWorld,
                                                const FString& LevelPackageName,
                                                FSimpleDelegate InOnLoadedDelegate,
                                                bool bWithSubLevels,
                                                bool bShowProgressDialog)
{
    check(InWorld);
    World = InWorld;
    OnLoadedDelegate = InOnLoadedDelegate;
    bLoadSubLevels = bWithSubLevels;

    LoadingLevelStreaming = nullptr;
    bool bSuccess = false;
    LoadingLevelStreaming = ULevelStreamingDynamic::LoadLevelInstance( World,
                                            LevelPackageName,
                                            FVector::ZeroVector,
                                            FRotator::ZeroRotator,
                                            bSuccess );
    if (!bSuccess)
    {
        return nullptr;
    }

    if (ProgressWindow == nullptr)
    {
        MakeProgressDialog();
    }

    if (bShowProgressDialog)
    {
        ShowProgressDialog();
    }

    return LoadingLevelStreaming;
}

ULevelStreamingDynamic* FLevelAsyncLoader::AsyncLoadLevel(UWorld* InWorld, 
                                    const TSoftObjectPtr<UWorld> InLevel,
                                    FSimpleDelegate InOnLoadedDelegate, 
                                    bool bWithSubLevels,
                                    bool bShowProgressDialog)
{
    check(InWorld);
    World = InWorld;
    OnLoadedDelegate = InOnLoadedDelegate;
    bLoadSubLevels = bWithSubLevels;

    LoadingLevelStreaming = nullptr;
    bool bSuccess = false;
    LoadingLevelStreaming = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr( World,
                                            InLevel,
                                            FVector::ZeroVector,
                                            FRotator::ZeroRotator,
                                            bSuccess );
    if (!bSuccess)
    {
        return nullptr;
    }

    if (ProgressWindow == nullptr)
    {
        MakeProgressDialog();
    }

    if (bShowProgressDialog)
    {
        ShowProgressDialog();
    }

    return LoadingLevelStreaming;
}

void FLevelAsyncLoader::Tick( float DeltaTime )
{
    if (LoadingLevelStreaming)
    {
        if (LoadingLevelStreaming->IsLevelLoaded() )
        {
            if (bLoadSubLevels)
            {
                ProcessSubLevels();
            }
            else
            {
                OnLoadCompleted();
            }
        }
    }
}

void FLevelAsyncLoader::OnLoadCompleted()
{
    LoadingLevelStreaming = nullptr;
    World = nullptr;
    OnLoadedDelegate = FSimpleDelegate();

    HideProgressDialog();
}

void FLevelAsyncLoader::ProcessSubLevels()
{
    bool bSuccess = false;

    FString LevelPackageName = LoadingLevelStreaming->GetWorldAssetPackageName();
    UPackage* LevelPackage = FindPackage( nullptr, *( LevelPackageName ) );
    if ( LevelPackage )
    {
        if (UWorld* PackageWorld = UWorld::FindWorldInPackage(LevelPackage))
        {
            TArray<ULevelStreaming*> Levels = PackageWorld->GetStreamingLevels();
            TArray<ULevelStreaming*> LinkLevels;
            for (ULevelStreaming* Level : Levels)
            {
                if (Level && PackageWorld->IsStreamingLevelBeingConsidered(Level))
                {
                    ULevelStreamingDynamic::LoadLevelInstance( World, 
                                                            Level->PackageNameToLoad.ToString(), 
                                                            FVector::ZeroVector,
                                                            FRotator::ZeroRotator,
                                                            bSuccess);
                }
            }
        }
    }

    OnLoadCompleted();
}

void FLevelAsyncLoader::MakeProgressDialog()
{
    // CreateWidget<UProgressBar>(World);
}

void FLevelAsyncLoader::ShowProgressDialog()
{
    // ProgressWindow->AddToViewport();
}

void FLevelAsyncLoader::HideProgressDialog()
{

    // ProgressWindow->RemoveFromViewport();
}

#undef LOCTEXT_NAMESPACE