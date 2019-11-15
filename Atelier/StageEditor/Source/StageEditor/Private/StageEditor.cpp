// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "StageEditor.h"
#include "StageEditorStyle.h"
#include "StageEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Logger.h"
#include "SStageEditorTab.h"
#include "StageTable.h"
#include "StageTableHelper.h"
#include "SStageEditorTab.h"
#include "StageEditorSettings.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "EditorUtilities.h"

DEFINE_LOG_CATEGORY(LogStageEditor)

static const FName StageEditorTabName("StageEditor");

#define LOCTEXT_NAMESPACE "FStageEditorModule"

void FStageEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FStageEditorStyle::Initialize();
	FStageEditorStyle::ReloadTextures();

	FStageEditorCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(FStageEditorCommands::Get().OpenPluginWindow,FExecuteAction::CreateRaw(this, &FStageEditorModule::OpenStageEditor),FCanExecuteAction());

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FStageEditorModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FStageEditorModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(StageEditorTabName, FOnSpawnTab::CreateRaw(this, &FStageEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FStageEditorTabTitle", "StageEditor"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FStageEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FStageEditorStyle::Shutdown();

	FStageEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(StageEditorTabName);
}

TSharedRef<SDockTab> FStageEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	LoadStageDataFromFile();

	TSharedRef<SDockTab> Tab = SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	.OnTabClosed_Raw(this, &FStageEditorModule::OnClosePluginTab)
	[
		SNew(SStageEditorTab, Proxy)
	];

	if (Proxy)
	{
		Proxy->StartupEditor();
	}
	return Tab;
}

void FStageEditorModule::OnClosePluginTab(TSharedRef<SDockTab> InDockTab)
{
    FEditorUtilities::StopEditorPlaySession();
	if (Proxy)
	{
		Proxy->ShutdownEditor();
	}
}

void FStageEditorModule::LoadStageDataFromFile()
{
	FStageTable* StageTable = new FStageTable();
	FString FilePath = GetDefault<UStageEditorSettings>()->StageDataFilePath.FilePath;
	FStageTableHelper::ImportStageDataFile(StageTable, FilePath);
	StateTablePtr = MakeShareable(StageTable);
}

void FStageEditorModule::OpenStageEditor()
{
	FGlobalTabmanager::Get()->InvokeTab(StageEditorTabName);
}

void FStageEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FStageEditorCommands::Get().OpenPluginWindow);
}

FStageEditorModule & FStageEditorModule::Get()
{
	FStageEditorModule& ASModule = FModuleManager::Get().LoadModuleChecked<FStageEditorModule>("StageEditor");
	return ASModule;
}

void FStageEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FStageEditorCommands::Get().OpenPluginWindow);
}

void FStageEditorModule::SaveStageDataToFile()
{
	auto PickConfigPath = [](const FString& Title) {
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		const FString DEFAULT_SAVE_PATH = FPaths::ProjectContentDir() + "Data/Maps/";
		FString CurrentFilename;

		const FString FileTypes = TEXT("Table CSV (*.csv)|*.csv");
		TArray<FString> OutFilenames;
		DesktopPlatform->SaveFileDialog(
			ParentWindowWindowHandle,
			Title,
			DEFAULT_SAVE_PATH,
			(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetBaseFilename(CurrentFilename) + TEXT(".csv"),
			FileTypes,
			EFileDialogFlags::None,
			OutFilenames
		);
		if (OutFilenames.Num() > 0)
		{
			return OutFilenames[0];
		}

		return FString();
	};

	FString FilePath = GetDefault<UStageEditorSettings>()->StageDataFilePath.FilePath;
	if (FilePath.IsEmpty())
	{
		FilePath = PickConfigPath(FString::Printf(TEXT("Save Stage as CSV... ")));
		GetMutableDefault<UStageEditorSettings>()->StageDataFilePath.FilePath = FilePath;
	}
	FStageTableHelper::ExportStageDataFile(StateTablePtr.Get(), FilePath);
}

UScriptStruct& FStageEditorModule::GetStageStruct()
{
	const TCHAR* STAGE_STRUCT_PATH = TEXT("/StageEditor/StageStruct.StageStruct");
	static UScriptStruct* StageStructType = FindObject<UScriptStruct>(ANY_PACKAGE, STAGE_STRUCT_PATH);
	if (!StageStructType)
	{
		StageStructType = Cast<UScriptStruct>(StaticLoadObject(UObject::StaticClass(), nullptr, STAGE_STRUCT_PATH, nullptr, LOAD_None, nullptr, true));
	}

	check(StageStructType);
	return *StageStructType;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStageEditorModule, StageEditor)
