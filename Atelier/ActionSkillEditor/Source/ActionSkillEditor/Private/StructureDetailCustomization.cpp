#include "StructureDetailCustomization.h"
#include "EditorStyleSet.h"
#include "Editor.h"
#include "Widgets/SLeafWidget.h"
#include "ModuleManager.h" 
#include "AssetToolsModule.h"
#include "IAssetTools.h"

#define LOCTEXT_NAMESPACE "ActionSkillEditor"

class SSimpleDoubleClickButton : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS( SSimpleDoubleClickButton )
    {}
        SLATE_EVENT( FOnClicked, OnClicked )
    SLATE_END_ARGS()

    void Construct( const FArguments& InArgs )
    {
        OnClicked = InArgs._OnClicked;
    }

    FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override
    {
        if (OnClicked.IsBound())
            return OnClicked.Execute();

        return FReply::Unhandled();
    }

private:
    FOnClicked OnClicked;
};

void FUserDefinedStructureDetails::CustomizeDetails(class IDetailLayoutBuilder& DetailLayout)
{
    const TArray<TWeakObjectPtr<UObject>>& Objects = DetailLayout.GetSelectedObjects();
    check(Objects.Num() > 0);

    if (Objects.Num() == 1)
    {
        UserDefinedStruct = CastChecked<UUserDefinedStruct>(Objects[0].Get());

        // IDetailCategoryBuilder& StructureCategory = DetailLayout.EditCategory("Structure", LOCTEXT("StructureCategory", "Structure"));
        IDetailCategoryBuilder& StructureCategory = DetailLayout.EditCategory(UserDefinedStruct->GetFName(), UserDefinedStruct->GetDisplayNameText());
        TSharedRef<SWidget> CategoryWidget =
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Right)
            .VAlign(VAlign_Center)
            .Padding(FMargin(4.0f))
            [
                SNew(SButton)
                // .Text(FText::FromString("Rename"))
                .OnClicked(this, &FUserDefinedStructureDetails::OnClickRenameStructure, Objects[0])
                .Content()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Rename"))
                    .Font(IDetailLayoutBuilder::GetDetailFontItalic())
                ]
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Right)
            .VAlign(VAlign_Center)
            .Padding(FMargin(4.0f))
            [
                SNew(SButton)
                .OnClicked(this, &FUserDefinedStructureDetails::OnOpenStructEditor)
                .Content()
                [
                    SNew(SImage)
                    .Image(FEditorStyle::GetBrush("BlueprintEditor.Details.ArgUpButton"))
                ]
            ];
        StructureCategory.HeaderContent(CategoryWidget);
        Layout = MakeShareable(new FUserDefinedStructureLayout(SharedThis(this)));
        StructureCategory.AddCustomBuilder(Layout.ToSharedRef());
    }
}

void FUserDefinedStructureLayout::GenerateChildContent( IDetailChildrenBuilder& ChildrenBuilder )
{
    const float NameWidth = 80.0f;
    const float ContentWidth = 130.0f;

    ChildrenBuilder.AddCustomRow(FText::GetEmpty())
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .MaxWidth(NameWidth)
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(this, &FUserDefinedStructureLayout::OnGetStructureStatus)
            .ToolTipText(this, &FUserDefinedStructureLayout::GetStatusTooltip)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .HAlign(HAlign_Left)
        [
            SNew(SBox)
            .WidthOverride(ContentWidth)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .Text(LOCTEXT("NewStructureField", "New Variable"))
                .OnClicked(this, &FUserDefinedStructureLayout::OnAddNewField)
            ]
        ]
    ];

    ChildrenBuilder.AddCustomRow(FText::GetEmpty())
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .MaxWidth(NameWidth)
        .HAlign(HAlign_Left)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Tooltip", "Tooltip"))
            .Font(IDetailLayoutBuilder::GetDetailFont())
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .HAlign(HAlign_Left)
        [
            SNew(SBox)
            .WidthOverride(ContentWidth)
            [
                SNew(SEditableTextBox)
                .Text(this, &FUserDefinedStructureLayout::OnGetTooltipText)
                .OnTextCommitted(this, &FUserDefinedStructureLayout::OnTooltipCommitted)
                .Font(IDetailLayoutBuilder::GetDetailFont())
            ]
        ]
    ];

    auto StructureDetailsSP = StructureDetails.Pin();
    if(StructureDetailsSP.IsValid())
    {
        if(auto Struct = StructureDetailsSP->GetUserDefinedStruct())
        {
            auto& VarDescArrayRef = FStructureEditorUtils::GetVarDesc(Struct);
            for (int32 Index = 0; Index < VarDescArrayRef.Num(); ++Index)
            {
                auto& VarDesc = VarDescArrayRef[Index];
                uint32 PositionFlag = 0;
                PositionFlag |= (0 == Index) ? EMemberFieldPosition::MFP_First : 0;
                PositionFlag |= ((VarDescArrayRef.Num() - 1) == Index) ? EMemberFieldPosition::MFP_Last : 0;
                TSharedRef<class FUserDefinedStructureFieldLayout> VarLayout = MakeShareable(new FUserDefinedStructureFieldLayout(StructureDetails,  SharedThis(this), VarDesc.VarGuid, PositionFlag));
                ChildrenBuilder.AddCustomBuilder(VarLayout);
            }
        }
    }
}

void FUserDefinedStructureDetails::PostChange(const class UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
    if (Struct && (GetUserDefinedStruct() == Struct))
    {
        if (Layout.IsValid())
        {
            Layout->OnChanged();
        }
    }
}


FReply FUserDefinedStructureDetails::OnClickRenameStructure(TWeakObjectPtr<UObject> InObject)
{
    if (!InObject.IsValid()) return FReply::Unhandled();

    const FVector2D DEFAULT_WINDOW_SIZE = FVector2D( 300, 100 );

    FString AssetRawName = InObject->GetName();

    TSharedRef< SWindow > RenameAssetWindow = SNew( SWindow )
        .Title( FText::FromString( "Rename " + AssetRawName ) )
        .ClientSize( DEFAULT_WINDOW_SIZE );

    TSharedRef< SEditableTextBox > NameEditablText = 
            SNew(SEditableTextBox)
            .Text(FText::FromString(AssetRawName))
            .Font(IDetailLayoutBuilder::GetDetailFont());

    TSharedRef< SBorder > RenameDialog =
        SNew(SBorder)
        .Padding(FMargin(10))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(STextBlock)
                .Text(FText::FromString("New Name: "))
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 10, 0, 0)
            [
                NameEditablText
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .VAlign(VAlign_Bottom)
            .Padding(0, 20, 0, 0)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(0.5f)
                [
                    SNew(SBox)
                    [
                        SNew(SBox)
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        .WidthOverride(100)
                        [
                            SNew(SButton)
                            .HAlign(HAlign_Fill)
                            .VAlign(VAlign_Fill)
                            .Text(LOCTEXT("RenameStructure_OK", "OK"))
                            .OnClicked_Lambda([this, RenameAssetWindow, InObject, NameEditablText]()
                            {
                                IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

                                TArray<FAssetRenameData> AssetsAndNames;
                                const FString PackagePath = FPackageName::GetLongPackagePath(InObject->GetOutermost()->GetName());
                                new(AssetsAndNames) FAssetRenameData(InObject.Get(), PackagePath, NameEditablText->GetText().ToString());
                                AssetTools.RenameAssets(AssetsAndNames);

                                RenameAssetWindow->RequestDestroyWindow();
                                return FReply::Handled();
                            })
                        ]
                    ]
                ]

                + SHorizontalBox::Slot()
                .FillWidth(0.5f)
                [
                    SNew(SBox)
                    [
                        SNew(SBox)
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        .WidthOverride(100)
                        [
                            SNew(SButton)
                            .HAlign(HAlign_Fill)
                            .VAlign(VAlign_Fill)
                            .Text(LOCTEXT("RenameStructure_Cancel", "Cancel"))
                            .OnClicked_Lambda([this, RenameAssetWindow]() {
                                RenameAssetWindow->RequestDestroyWindow();
                                return FReply::Handled();
                            })
                        ]
                    ]
                ]
            ]
        ];

    RenameAssetWindow->SetContent( RenameDialog );

    GEditor->EditorAddModalWindow( RenameAssetWindow );

    return FReply::Handled();
}

FReply FUserDefinedStructureDetails::OnOpenStructEditor()
{
    IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
    TArray<UObject*> Assets;
    Assets.Add(UserDefinedStruct.Get());
    AssetTools.OpenEditorForAssets(Assets);

    return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE