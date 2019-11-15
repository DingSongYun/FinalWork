#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "SStageModeBase.h"
#include "StageTable.h"

struct FStageItem
{
public:
	FStageItem(FStageScopePtr Stage);

	FString GetStageName();

	FStageScopePtr GetData() { return StageScope; };

private:
	FStageScopePtr StageScope;
};

typedef TSharedPtr<FStageItem> FStageItemPtr;
typedef SListView<FStageItemPtr> SStageListView;

DECLARE_DELEGATE_OneParam(FOnChoosenItem, FStageScopePtr/*Stage*/)

class SStageListMode : public SStageModeBase
{
public:
	SLATE_BEGIN_ARGS(SStageListMode) {}
	SLATE_EVENT(FOnChoosenItem, OnChoosenItem)
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs);
	void CollectStageData(bool ReBuildList);
	TSharedRef<ITableRow> GeneraterStageItemWidget(FStageItemPtr InItem, const TSharedRef<STableViewBase>& OwnerTable);

	FString GetName() override { return FString("Stage List"); }
	FString GetDescription() override { return FString("Show all of stage info"); }

	void OnSearchStage(const FText& InFilterText);

private:
	TArray<FStageItemPtr> StageListSource;
	TSharedPtr<SStageListView> StageListView;
	FOnChoosenItem OnChoosenItem;
private:
	void NewStage();
	void EditStage(FStageItemPtr StageItem);
	void DelStage(FStageItemPtr StageItem);
};