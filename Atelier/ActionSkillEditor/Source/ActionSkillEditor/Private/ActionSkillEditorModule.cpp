// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-19

#include "ActionSkillEditorModule.h"
#include "ActionSkillEditor.h"
#include "ISkillPreviewProxy.h"
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"
#include "UserStructOnScope.h"
#include "CustomLayout/UserStructOnScopeDetailsCustomization.h"
#include "CustomLayout/UserStructOnScopePropCustomLayout.h"

#define LOCTEXT_NAMESPACE "ActionSkillEditorModule"

EAssetTypeCategories::Type ASAssetTypesCategory = EAssetTypeCategories::Misc;

class FActionSkillEditorModule : public IActionSkillEditorModule
{
private:
	FSimpleMulticastDelegate OpenEditorDelegate;
	FSimpleMulticastDelegate EditorInitializedDelegate;
	FSimpleMulticastDelegate CloseEditorDelegate;
	TWeakPtr<FActionSkillEditor> ActionSkillEditorInst;
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		ASAssetTypesCategory = AssetTools.RegisterAdvancedAssetCategory("ActionSkillTypes", LOCTEXT("ActionSkillEditor", "ActionSkillTypes"));
		RegisterCustomLayout();
	}

	virtual void ShutdownModule() override
	{
		UnregisterCustomLayout();
	}

	void RegisterCustomLayout()
	{
		FPropertyEditorModule& PropertyEditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		// PropertyEditModule.RegisterCustomClassLayout(FUserStructOnScope::StaticStruct()->GetFName(), 
		// 	FOnGetDetailCustomizationInstance::CreateStatic(&FUserStructOnScopeDetailsCustomization::MakeInstance));

		PropertyEditModule.RegisterCustomPropertyTypeLayout(FUserStructOnScope::StaticStruct()->GetFName(), 
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUserStructOnScopePropCustomLayout::MakeInstance)
		);
	}

	void UnregisterCustomLayout()
	{
		FPropertyEditorModule& PropertyEditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditModule.UnregisterCustomPropertyTypeLayout(FUserStructOnScope::StaticStruct()->GetFName());
	}

	virtual TSharedRef<class FActionSkillEditor> CreateActionSkillEditor() override
	{
		TSharedRef<FActionSkillEditor> ActionSKillEditor(new FActionSkillEditor());
		ActionSKillEditor->InitializeEditor();
		ActionSkillEditorInst = ActionSKillEditor;
		return ActionSKillEditor;
	}

	virtual TWeakPtr<FActionSkillEditor> GetSingletonSkillEditor() override
	{
		return ActionSkillEditorInst;
	}

	FSimpleMulticastDelegate& OnOpenEditorDelegate() override { return OpenEditorDelegate; }
	FSimpleMulticastDelegate& OnCloseEditorDelegate() override { return CloseEditorDelegate; }
	FSimpleMulticastDelegate& OnEditorInitializedDelegate() override { return EditorInitializedDelegate; }
};

uint32 IActionSkillEditorModule::GetAdvanceAssetTypeCategory()
{
	return ASAssetTypesCategory;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FActionSkillEditorModule, ActionSkillEditor)