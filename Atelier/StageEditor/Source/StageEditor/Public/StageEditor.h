// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-16

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IStageEditorInterface.h"
#include "DelegateCombinations.h"

class FToolBarBuilder;
class FMenuBuilder;
class FStageTable;

class FStageEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void OpenStageEditor();

	template<class T, typename std::enable_if<std::is_base_of<IStageEditorInterface, T>::value, int>::type = 0>
	
	void RegisterProxyInteface() { Proxy = new T(); }
	IStageEditorInterface* GetProxyInterface() { return Proxy; }

	TSharedPtr<FStageTable> GetStateTable() { return StateTablePtr; }

	void SaveStageDataToFile();

	/** Stage 数据结构 */
	static UScriptStruct& GetStageStruct();
	static FStageEditorModule& Get();

private:
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	void OnClosePluginTab(TSharedRef<class SDockTab> InDockTab);

	void LoadStageDataFromFile();

private:
	TSharedPtr<class FUICommandList>                    PluginCommands;
	IStageEditorInterface*                              Proxy;
	TSharedPtr<FStageTable>                             StateTablePtr;
};
