#include "EditorUtilities.h"
#include "PackageName.h"
#include "FileHelpers.h"
#include "Paths.h"
#include "Editor.h"
#include "SlateApplication.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "GameplayTagsManager.h"
#include "Kismet/GABlueprintFunctionLibrary.h"
#include "EditorLevelUtils.h"
#include "SMapEditorToolbar.h"

void FEditorUtilities::OpenLevelByName(const FString& LevelName) {
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	bool isCurrent = EditorWorld->GetMapName().Equals(LevelName);
	if (isCurrent)
		return;
	/*
	bool isCurrentMapEditor = EditorWorld->GetMapName().Equals("MapEditor");
	if (isCurrentMapEditor)
	{
		TSharedPtr<SMapEditorToolbar> mapEditorToolBar = StaticCastSharedPtr<SMapEditorToolbar>(FGlobalTabmanager::Get()->FindExistingLiveTab(MapEditorTabName));
		if (mapEditorToolBar)
		{
			mapEditorToolBar->CleanAll();
		}
	}
	*/

	FEditorUtilities::CleanCurrentLevel(true, true);
	UGABlueprintFunctionLibrary::ReloadEditorLuaIfEdAvaliable();
	const FString FileToOpen = FPackageName::LongPackageNameToFilename(PATH_ROOT_MAPS + LevelName, FPackageName::GetMapPackageExtension());
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

bool FEditorUtilities::DumpRowToDataTable(UDataTable* ToDataTable, FName RowName, const void* RowData)
{
	if (ToDataTable == nullptr)
	{
		return false;
	}

	{ // Local Add Row
		FTableRowBase *rowbase = (FTableRowBase*)RowData;
		ToDataTable->AddRow(RowName, *rowbase);
	}

	ToDataTable->MarkPackageDirty();

	UPackage* Package = ToDataTable->GetOutermost();

	if (Package)
	{
		FString FilePath = FString::Printf(TEXT("%s%s"), *Package->GetPathName(), *FPackageName::GetAssetPackageExtension());
		return UPackage::SavePackage(Package, ToDataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
	}

	return false;
}

void FEditorUtilities::CleanCurrentLevel(bool CleanLoadedMap, bool CleanConfigObjs)
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	check(EditorWorld != nullptr);

	if (CleanLoadedMap)
	{
		TArray<class ULevelStreaming*> StreamingLevels = EditorWorld->GetStreamingLevels();
		TArray<class ULevelStreaming*> LevelToRemove;

		for (int Index = 0; Index < StreamingLevels.Num(); ++Index)
		{
			ULevelStreaming* Level = StreamingLevels[Index];

			if (Level->GetLoadedLevel()->IsPersistentLevel())
				continue;
			LevelToRemove.Add(Level);
		}
		for (ULevelStreaming* Level : LevelToRemove)
		{
			Level->bLocked = false;
			Level->GetLoadedLevel()->bLocked = false;
			EditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
		}
	}

	if (CleanConfigObjs)
	{
		ULevel* PersistentLevel = EditorWorld->PersistentLevel;
		for (AActor* Actor : PersistentLevel->Actors)
		{
			if (Actor != nullptr && Actor->Implements<UStageObject>())
			{
				Actor->Destroy();
				EditorWorld->RemoveActor(Actor, true);
			}
		}
		PersistentLevel->Modify();
		PersistentLevel->Model->Modify();
	}

	//EditorWorld->CleanupWorld();
	EditorWorld->CleanupActors();
	GEditor->NoteSelectionChange();
}