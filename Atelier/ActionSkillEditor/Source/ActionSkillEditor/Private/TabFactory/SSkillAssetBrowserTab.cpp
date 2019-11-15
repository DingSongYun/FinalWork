#include "SSkillAssetBrowserTab.h"
#include "IContentBrowserSingleton.h"
#include "Widgets/Layout/SBorder.h"
#include "ContentBrowserModule.h"
#include "FrontendFilterBase.h"
#include "Widgets/SBoxPanel.h"
#include "ActionSkillEditor.h"
#include "ModuleManager.h"
#include "EditorStyle.h"
#include "Editor.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimationAsset.h"

#define LOCTEXT_NAMESPACE "SkillAssetBrowser"

class FFrontendFilter_CharaAnims : public FFrontendFilter
{
public:
    FFrontendFilter_CharaAnims(TSharedPtr<FFrontendFilterCategory> InCategory, TWeakPtr<class FActionSkillEditor> InActionSkillEditor) 
        : FFrontendFilter(InCategory)
        , ActionSkillEditor(InActionSkillEditor)
    {}

    static FString TrimLastFolder(const FString& RawPath)
    {
        int32 Index = -1;

        return RawPath.FindLastChar('/', Index) ? RawPath.Left(Index) : RawPath;
    }

    USkeletalMesh* GetCharacterMesh() const
    {
        ACharacter* SkillCharacter = ActionSkillEditor.Pin()->GetPreviewSkillCaster();
        if (SkillCharacter)
        {
            if (USkeletalMeshComponent* MeshComp = SkillCharacter->GetMesh())
            {
                return MeshComp->SkeletalMesh;
            }
        }

        return nullptr;
    }
protected:
    TWeakPtr<class FActionSkillEditor> ActionSkillEditor;
};

class FFrontendFilter_CharaCommonAnims : public FFrontendFilter_CharaAnims
{
public:
    FFrontendFilter_CharaCommonAnims(TSharedPtr<FFrontendFilterCategory> InCategory, TWeakPtr<class FActionSkillEditor> InActionSkillEditor) 
        : FFrontendFilter_CharaAnims(InCategory, InActionSkillEditor)
    {}

    // FFrontendFilter implementation
    virtual FString GetName() const override { return TEXT("CharaPrivateAnims"); }
    virtual FText GetDisplayName() const override { return LOCTEXT("FFrontendFilter_CharaCommonAnims", "角色通用动画"); }
    virtual FText GetToolTipText() const override { return LOCTEXT("FFrontendFilter_CharaCommonAnims_ToolTips", "显示角色基础通用动画."); }

    // IFilter implementation
    virtual bool PassesFilter(FAssetFilterType InItem) const override
    {
        if (auto SkeletalMesh = GetCharacterMesh())
        {
            USkeleton* Skeleton = SkeletalMesh->Skeleton;
            FAssetData SkeletonAsset(Skeleton);

            const FString SkeSuitPath = TrimLastFolder(SkeletonAsset.PackagePath.ToString());
            const FString AssetSuitPath = TrimLastFolder(InItem.PackagePath.ToString());

            return AssetSuitPath == SkeSuitPath;
        }
        return false;
    }
};

class FFrontendFilter_CharaPrivateAnims : public FFrontendFilter_CharaAnims
{
public:
    FFrontendFilter_CharaPrivateAnims(TSharedPtr<FFrontendFilterCategory> InCategory, TWeakPtr<class FActionSkillEditor> InActionSkillEditor)
        : FFrontendFilter_CharaAnims(InCategory, InActionSkillEditor)
    {}

    // FFrontendFilter implementation
    virtual FString GetName() const override { return TEXT("CharaCommonAnims"); }
    virtual FText GetDisplayName() const override { return LOCTEXT("FFrontendFilter_CharaPrivateAnims", "角色私有动画"); }
    virtual FText GetToolTipText() const override { return LOCTEXT("FFrontendFilter_CharaPrivateAnims_ToolTips", "显示角色私有动画."); }

    // IFilter implementation
    virtual bool PassesFilter(FAssetFilterType InItem) const override
    {
        if (auto SkeletalMesh = GetCharacterMesh())
        {
            FAssetData MeshAsset(SkeletalMesh);

            const FString MeshSuitPath = TrimLastFolder(MeshAsset.PackagePath.ToString());
            const FString AssetSuitPath = TrimLastFolder(InItem.PackagePath.ToString());

            return AssetSuitPath == MeshSuitPath;
        }
        return false;
    }
};

void SSkillAssetBrowserTab::Construct(const FArguments& InArgs, TWeakPtr<class IActionSkillEditor> InEditorPtr)
{
    ActionSkillEditor = StaticCastSharedPtr<FActionSkillEditor>(InEditorPtr.Pin());

    FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

    FAssetPickerConfig Config;
    {
        // Init Config
        Config.InitialAssetViewType = EAssetViewType::Column;
        Config.bAddFilterUI = true;
        Config.bShowPathInColumnView = true;
        Config.bSortByPathInColumnView = true;
        Config.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SSkillAssetBrowserTab::HandleFilterAsset);
        Config.DefaultFilterMenuExpansion = EAssetTypeCategories::Animation;
        // Config Filter
        Config.Filter.bRecursiveClasses = true;
        Config.Filter.ClassNames.Add(UAnimationAsset::StaticClass()->GetFName());

        // Asset Actions
        Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SSkillAssetBrowserTab::OnRequestOpenAsset);

        // Extera Filter
        {
            ACharacter* SkillCharacter = ActionSkillEditor.Pin()->GetPreviewSkillCaster();
            USkeletalMeshComponent* MeshComp = SkillCharacter ? SkillCharacter->GetMesh() : nullptr;

            TSharedPtr<FFrontendFilterCategory> AnimCategory = MakeShareable( new FFrontendFilterCategory(LOCTEXT("CharacterAnimationFilters", "Anim Filters"), LOCTEXT("CharacterAnimationFiltersTooltip", "Filter assets by all filters in this category.")) );
            Config.ExtraFrontendFilters.Add( MakeShareable(new FFrontendFilter_CharaCommonAnims(AnimCategory, ActionSkillEditor)) );
            Config.ExtraFrontendFilters.Add( MakeShareable(new FFrontendFilter_CharaPrivateAnims(AnimCategory, ActionSkillEditor)) );
        }
    }

    this->ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SBorder)
            .Padding(FMargin(3))
            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            [
                ContentBrowserModule.Get().CreateAssetPicker(Config)
            ]
        ]
    ];
}

bool SSkillAssetBrowserTab::HandleFilterAsset(const FAssetData& InAssetData) const
{
    bool bShouldFilter = false;

    if (InAssetData.AssetName.ToString().StartsWith("__"))
    {
        return true;
    }

    if (InAssetData.GetClass()->IsChildOf(UAnimationAsset::StaticClass()))
    {
        USkeleton* DesiredSkeleton = GetSkillCharacterSkeleton();
        if (DesiredSkeleton)
        {
            FString SkeletonString = FAssetData(DesiredSkeleton).GetExportTextName();

            bShouldFilter = (InAssetData.TagsAndValues.FindRef(TEXT("Skeleton")) != SkeletonString);
        }

        if (!bShouldFilter)
        {

        }
    }

    return bShouldFilter;
}

USkeleton* SSkillAssetBrowserTab::GetSkillCharacterSkeleton() const
{
    ACharacter* SkillCharacter = ActionSkillEditor.Pin()->GetPreviewSkillCaster();
    if (SkillCharacter)
    {
        if (USkeletalMeshComponent* MeshComp = SkillCharacter->GetMesh())
        {
            if(MeshComp->SkeletalMesh) return MeshComp->SkeletalMesh->Skeleton;
        }
    }

    return nullptr;
}

void SSkillAssetBrowserTab::OnRequestOpenAsset(const FAssetData& AssetData)
{
    if (UObject* ObjectToEdit = AssetData.GetAsset())
    {
        GEditor->EditObject( ObjectToEdit );
    }
}

#undef LOCTEXT_NAMESPACE