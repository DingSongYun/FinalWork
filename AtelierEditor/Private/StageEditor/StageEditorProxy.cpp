#include "StageEditorProxy.h"
#include "EditorStaticLibrary.h"
#include "Kismet/GABlueprintFunctionLibrary.h"
#include "LevelEditor.h"
#include "Engine/World.h"
#include "Widgets/SWeatherSelectionWidget.h"
#include "IDetailsView.h"
#include "StageCustomDetail.h"
#include "Types/WeatherStruct.h"
#include "StageConfigStruct.h"

#define EDITOR_IN_PIE 1
ULuaStageEdProxy* FStageEditorProxy::sLuaEditorPtr = nullptr;

namespace LuaEdInterface
{
    const FString AssembleCharacter = "K2_AssembleCharacter";
    const FString InitializeEditor = "K2_InitializeEditor";
    const FString SetWeather = "K2_SetWeather";
    const FString NotifyMapChanged = "K2_OnStageMapChanged";
};

void ULuaStageEdProxy::AssembleCharacter(class ACharacter* InCharacter, int32 NpcId)
{
    check(InCharacter);
    TArray<FLuaBPVar> Args {
        ULuaBlueprintLibrary::CreateVarFromObject(InCharacter),
        ULuaBlueprintLibrary::CreateVarFromInt(NpcId)
    };
    CallLuaFunction_Call(LuaEdInterface::AssembleCharacter, Args, true);
}

void ULuaStageEdProxy::InitializeEditor()
{
    TArray<FLuaBPVar> Args;
    Args.Add(ULuaBlueprintLibrary::CreateVarFromObject(this));
    CallLuaFunction_Call(LuaEdInterface::InitializeEditor, Args, true);
}

void ULuaStageEdProxy::SetWeather(uint32 WeatherId, const FName& WeatherName)
{
    TArray<FLuaBPVar> Args;
    Args.Add(ULuaBlueprintLibrary::CreateVarFromInt(WeatherId));
    Args.Add(ULuaBlueprintLibrary::CreateVarFromString(WeatherName.ToString()));
    CallLuaFunction_Call(LuaEdInterface::SetWeather, Args, true);
}

void ULuaStageEdProxy::OnStageMapChanged()
{
    TArray<FLuaBPVar> Args;
    CallLuaFunction_Call(LuaEdInterface::NotifyMapChanged, Args, true);
}

// FDelegateHandle OnMapChangedHandler;
FStageEditorProxy::FStageEditorProxy()
{
    // FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    // OnMapChangedHandler = LevelEditorModule.OnMapChanged().AddRaw(this, &FStageEditorProxy::OnMapChanged);
}

FStageEditorProxy::~FStageEditorProxy()
{
    // FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    // LevelEditorModule.OnMapChanged().Remove(OnMapChangedHandler);
}

void FStageEditorProxy::StartupEditor()
{}

void FStageEditorProxy::ShutdownEditor()
{
    ExitEditorContext();
}

void FStageEditorProxy::SetWeather(uint32 WeatherId, const FName& WeatherName)
{
    if (sLuaEditorPtr)
    {
        sLuaEditorPtr->SetWeather(WeatherId, WeatherName);
    }
}

TSharedRef<class SWidget> FStageEditorProxy::BuildExternalPanel()
{
    TSharedRef<SVerticalBox> WeatherPanel = SNew( SVerticalBox );
    WeatherPanel->AddSlot()
    .AutoHeight()
    [
        SNew(SWeatherSelectionWidget)
        .OnSelectionChanged_Lambda([this](FWeatherEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo) 
        {
            SetWeather(InSelectedItem->GetWeatherId(), InSelectedItem->GetWeatherName());
        })
    ];

    return WeatherPanel;
}

void FStageEditorProxy::OnStageMapChanged()
{
    if (sLuaEditorPtr)
    {
        sLuaEditorPtr->OnStageMapChanged();
    }
}

void FStageEditorProxy::CustomStageDetailLayout(IDetailsView* DetailsView)
{
    DetailsView->RegisterInstancedCustomPropertyTypeLayout
	(
		FStageWeather::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWeatherDetailsCustomization::MakeInstance)
	);
	
	DetailsView->RegisterInstancedCustomPropertyTypeLayout
	(
		FStageConfig::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FConfigDetailsCustomization::MakeInstance)
	);
}

void FStageEditorProxy::InitEditorContext()
{
#if !EDITOR_IN_PIE
    UGABlueprintFunctionLibrary::StartEditorLua();
    if (!sLuaEditorPtr)
    {
        sLuaEditorPtr = NewObject<ULuaStageEdProxy>();
        sLuaEditorPtr->SetLuaBPVar_Call(
            UEditorStaticLibrary::CallToLua("getStageEditor")
        );
        sLuaEditorPtr->InitializeEditor();
        sLuaEditorPtr->AddToRoot();
    }
#else
    UEditorStaticLibrary::CallToLua("createStageEditor");
#endif
    if (!sLuaEditorPtr)
    {
        sLuaEditorPtr = NewObject<ULuaStageEdProxy>();
        sLuaEditorPtr->SetLuaBPVar_Call(
            UEditorStaticLibrary::CallToLua("getStageEditor")
        );
        sLuaEditorPtr->InitializeEditor();
        sLuaEditorPtr->AddToRoot();
    }
    bInitialized = true;
}

void FStageEditorProxy::ExitEditorContext()
{
    OnExitEditorContext();
}

void FStageEditorProxy::OnExitEditorContext()
{
#if !EDITOR_IN_PIE
    UGABlueprintFunctionLibrary::StopEditorLua();
#endif
    UGABlueprintFunctionLibrary::StopEditorLua();
    if (sLuaEditorPtr)
    {
        sLuaEditorPtr->RemoveFromRoot();
        sLuaEditorPtr = nullptr;
    }
    bInitialized = false;
}

void FStageEditorProxy::AddReferencedObjects(FReferenceCollector& Collector)
{
}

UWorld* FStageEditorProxy::GetWorld()
{
    FWorldContext* PIEWorldContext = GEditor->GetPIEWorldContext();
    UWorld* World = PIEWorldContext ? PIEWorldContext->World() : GEditor->GetEditorWorldContext().World();
    check(World);
    return World;
}