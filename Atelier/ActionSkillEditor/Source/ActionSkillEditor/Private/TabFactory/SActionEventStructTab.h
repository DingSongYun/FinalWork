// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-08-26
#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SSingleObjectDetailsPanel.h"

struct FAssetData;
class ITableRow;
class STableViewBase;
class FActionSkillDefineMode;

class SActionEventStructTab : public SCompoundWidget
{
public:
    typedef UObject* StructureTypePtr;
    typedef SListView<StructureTypePtr> SEventStructListType;
    SLATE_BEGIN_ARGS(SActionEventStructTab){}

    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs, FActionSkillDefineMode* DefineMode);

private:
    void CollectionActionEventStructure();
    TSharedRef<ITableRow> GenerateMenuItemRow( StructureTypePtr InItem, const TSharedRef<STableViewBase>& OwnerTable);
    void OnNewAsset(const FAssetData& InAssetData) { InternalOnAssetModified(InAssetData); }
    void OnDeleteAsset(const FAssetData& InAssetData) { InternalOnAssetModified(InAssetData); }
    void OnRenameAsset(const FAssetData& InAssetData, const FString& InNewName) { InternalOnAssetModified(InAssetData); }
    void InternalOnAssetModified(const FAssetData& InAssetData);
    void RebuildEventStructureList();

private:
    /** Pointer to FActionSkillDefineMode. */
    FActionSkillDefineMode* SkillEdDefineModePtr;

    TArray< StructureTypePtr > OptionSource;

    TSharedPtr< SEventStructListType > EventStructureListView;
};