#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "EditorConfig.h"
#include "UnrealEd.h"

class SMapEditorToolbar;
class FEditorTool;
class SAccessoriesEditorToolbar;
class SInteractiveEditorToolbar;
class SAvatarEditorToolbar;
class SWorkshopEditorToolbar;
class SFurnitureEditorToolbar;

class FAtelierEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	TSharedRef<SDockTab> CreateMapEditorTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> CreateAccessoriesEditorTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> CreateInteractiveEditorTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> CreateAvatarEditorTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> CreateWorkshopEditorTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> CreateFurnitureEditorTab(const FSpawnTabArgs& Args);

	void RegisterCustomDetail();

	TArray<TUniquePtr<FEditorTool>> EditorTools;
	TWeakPtr<SMapEditorToolbar> MapEditorToolbar;

	void RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer);
public:
	UEditorConfig* Config;
	TWeakPtr<SAccessoriesEditorToolbar> AccessoriesEditorToolbar;
	TWeakPtr<SInteractiveEditorToolbar> InteractiveEditorToolbar;
	TWeakPtr<SAvatarEditorToolbar> AvatarEditorToolbar;
	TWeakPtr<SWorkshopEditorToolbar> WorkshopEditorToolbar;
	TWeakPtr<SFurnitureEditorToolbar> FurnitureEditorToolbar;

	TArray<FName> RegisteredComponentClassNames;
};
