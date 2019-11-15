#include "EditorUtilities.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "SlateApplication.h"
#include "EditorLevelUtils.h"
#include "PackageName.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "Paths.h"
#include "LevelEditor.h"
#include "ILevelViewport.h"
#include "LevelEditorViewport.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

void FEditorUtilities::OpenLevelByName(const FString& LevelName) {
    UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
    bool isCurrent = EditorWorld->GetMapName().Equals(LevelName);
    if (isCurrent)
        return;

    const FString FileToOpen = FPackageName::LongPackageNameToFilename(LevelName, FPackageName::GetMapPackageExtension());
    const bool bLoadAsTemplate = false;
    const bool bShowProgress = true;
    FEditorFileUtils::LoadMap(FileToOpen, bLoadAsTemplate, bShowProgress);
}

FString FEditorUtilities::OpenPickFileDialog(const FString& RootDir, const FString& DialogTitle, const FString& Types)
{
    static void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
    static IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

    TArray<FString> OutFiles;

    bool Ret = DesktopPlatform->OpenFileDialog(
        ParentWindowPtr,
        DialogTitle,
        RootDir,
        TEXT(""),
        Types,
        EFileDialogFlags::None,
        OutFiles
    );

    if (Ret && OutFiles.Num() > 0)
    {
        return OutFiles[0];
    }

    return "";
}

FString FEditorUtilities::OpenSaveFileDialog(const FString& RootDir, const FString& DialogTitle, const FString& Types)
{
    static void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
    static IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

    TArray<FString> OutFiles;

    bool Ret = DesktopPlatform->SaveFileDialog(
        ParentWindowPtr,
        DialogTitle,
        RootDir,
        TEXT(""),
        Types,
        EFileDialogFlags::None,
        OutFiles
    );

    if (Ret && OutFiles.Num() > 0)
    {
        return OutFiles[0];
    }

    return "";
}

void FEditorUtilities::PlayInEditor()
{
    FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>( TEXT("LevelEditor") );

    TSharedPtr<ILevelViewport> ActiveLevelViewport = LevelEditorModule.GetFirstActiveViewport();

    const bool bAtPlayerStart = true;
    const bool bSimulateInEditor = false;

    if (ActiveLevelViewport.IsValid() && FSlateApplication::Get().FindWidgetWindow(ActiveLevelViewport->AsWidget()).IsValid())
    {
        const FVector* StartLoc = NULL;
        const FRotator* StartRot = NULL;
        if( !bAtPlayerStart )
        {
            StartLoc = &ActiveLevelViewport->GetLevelViewportClient().GetViewLocation();
            StartRot = &ActiveLevelViewport->GetLevelViewportClient().GetViewRotation();
        }

        const bool bUseMobilePreview = false;
        const int32 DestinationConsoleIndex = -1;

        GUnrealEd->RequestPlaySession( bAtPlayerStart, ActiveLevelViewport, bSimulateInEditor, StartLoc, StartRot, DestinationConsoleIndex, bUseMobilePreview );
    }
    else
    {
        GUnrealEd->RequestPlaySession( bAtPlayerStart, NULL, bSimulateInEditor );
    }
}


void FEditorUtilities::StopEditorPlaySession()
{
    FWorldContext* PIEWorldContext = GEditor->GetPIEWorldContext();
    //UWorld* World = PIEWorldContext ? PIEWorldContext->World() : GEditor->GetEditorWorldContext().World();
    if (PIEWorldContext)
    {
        GUnrealEd->TeardownPlaySession( *PIEWorldContext );
    }
}