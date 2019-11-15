// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-19
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogActionSkillEditor, Log, All)

class ISkillPreviewProxy;

class IActionSkillEditorModule : public IModuleInterface
{
public:
	virtual TSharedRef<class FActionSkillEditor> CreateActionSkillEditor() = 0;
	virtual TWeakPtr<class FActionSkillEditor> GetSingletonSkillEditor() = 0;

	DECLARE_DELEGATE_RetVal_OneParam(class ISkillPreviewProxy*, FGetSkillPreviewProxyDelegate, class UWorld* /*InWorld*/)
	virtual void  SetSkillPreviewProxy(FGetSkillPreviewProxyDelegate InProxy) 
	{
		GetSkillPreviewProxyDelegate = InProxy;
	}

	ISkillPreviewProxy* GetSkillPreviewProxy(class UWorld* InWorld) { return GetSkillPreviewProxyDelegate.Execute(InWorld);}

	virtual FSimpleMulticastDelegate& OnOpenEditorDelegate() = 0;
	virtual FSimpleMulticastDelegate& OnEditorInitializedDelegate() = 0;
	virtual FSimpleMulticastDelegate& OnCloseEditorDelegate() = 0;
	static uint32 GetAdvanceAssetTypeCategory();

protected:
	FGetSkillPreviewProxyDelegate GetSkillPreviewProxyDelegate;
};