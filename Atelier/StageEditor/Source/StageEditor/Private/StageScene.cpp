#include "IStageScene.h"
#include "UObject/UnrealType.h"
#include "LevelAsyncLoader.h"
#include "SoftLevelPtr.h"
#include "Editor.h"
#include "IStagePlacement.h"
#include "IStageEditorInterface.h"
#include "StageEditor.h"
#include "Misc/NotifyHook.h"
#include "EngineUtils.h"
#include "JsonObjectConverter.h"
#include "Logger.h"
#include "Paths.h"
#include "FileHelper.h"
#include "StageConfigStruct.h"
#include "StageEditorSettings.h"

const FString IDENTIFIER_CLASS = "_class";
const FString IDENTIFIER_TRANSFORM = "transform";
const FString IDENTIFIER_LOCATION = "position";
const FString IDENTIFIER_ROTATION = "rotation";
const FString IDENTIFIER_SCALE = "scale";

void VectorToJsonObject(const FVector& InLocation, TSharedRef<FJsonObject> OutJsonObject)
{
    OutJsonObject->SetNumberField("x", InLocation.X);
    OutJsonObject->SetNumberField("y", InLocation.Y);
    OutJsonObject->SetNumberField("z", InLocation.Z);
}

void RotatorToJsonObject(const FRotator& InRotator, TSharedRef<FJsonObject> OutJsonObject)
{
    OutJsonObject->SetNumberField("roll", InRotator.Roll);
    OutJsonObject->SetNumberField("pitch", InRotator.Pitch);
    OutJsonObject->SetNumberField("yaw", InRotator.Yaw);
}

void TransformToJsonObject(const FTransform& InTransform, TSharedRef<FJsonObject> OutJsonObject)
{
    // Location
    {
        TSharedRef<FJsonObject> LocationJson = MakeShared<FJsonObject>();
        VectorToJsonObject(InTransform.GetLocation(), LocationJson);
        OutJsonObject->SetObjectField(IDENTIFIER_LOCATION, LocationJson);
    }

    // Rotator
    {
        TSharedRef<FJsonObject> RotationJson = MakeShared<FJsonObject>();
        RotatorToJsonObject(InTransform.GetRotation().Rotator(), RotationJson);
        OutJsonObject->SetObjectField(IDENTIFIER_ROTATION, RotationJson);
    }

    // Scale
    {
        TSharedRef<FJsonObject> ScaleJson = MakeShared<FJsonObject>();
        VectorToJsonObject(InTransform.GetScale3D(), ScaleJson);
        OutJsonObject->SetObjectField(IDENTIFIER_SCALE, ScaleJson);
    }
}

FVector JsonObjectToVector(TSharedRef<FJsonObject> JsonObject)
{
    FVector Vector;
    Vector.X = JsonObject->GetNumberField("x");
    Vector.Y = JsonObject->GetNumberField("y");
    Vector.Z = JsonObject->GetNumberField("z");

    return Vector;
}

FRotator JsonObjectToRotator(TSharedRef<FJsonObject> JsonObject)
{
    FRotator Rotator;
    Rotator.Roll = JsonObject->GetNumberField("roll");
    Rotator.Pitch = JsonObject->GetNumberField("pitch");
    Rotator.Yaw = JsonObject->GetNumberField("yaw");
    return Rotator;
}

FTransform JsonObjectToTransform(TSharedRef<FJsonObject> JsonObject)
{
    FTransform Transform;
    Transform.SetLocation(
        JsonObjectToVector(JsonObject->GetObjectField(IDENTIFIER_LOCATION).ToSharedRef()));
    Transform.SetRotation(
        JsonObjectToRotator(JsonObject->GetObjectField(IDENTIFIER_ROTATION).ToSharedRef()).Quaternion());
    Transform.SetScale3D(
        JsonObjectToVector(JsonObject->GetObjectField(IDENTIFIER_SCALE).ToSharedRef()));

    return Transform;
}

struct FStageStructProxy
{
    void InitStage(FStageScopePtr InStruct)
    {
        InnerStruct = InStruct;
    }

    void Clean()
    {
        InnerStruct = nullptr;
    }

    FStageScopePtr GetStage()
    {
        return InnerStruct;
    }

	int32 GetId()
	{
		check(IsValid());
		if (UIntProperty* Prop = Cast<UIntProperty>(InnerStruct->GetStructPropertyByName("Id")))
		{
			return Prop->GetPropertyValue(Prop->ContainerPtrToValuePtr<void>(InnerStruct->GetData()->GetStructMemory()));
		}
		return 0;
	}


	FString GetName() 
	{
		check(IsValid());
		if (UNameProperty* Prop = Cast<UNameProperty>(InnerStruct->GetStructPropertyByName("Name")))
		{
			return Prop->GetPropertyValue(Prop->ContainerPtrToValuePtr<void>(InnerStruct->GetData()->GetStructMemory())).ToString();
		}
		return FString();
	}

    TSoftObjectPtr<UWorld> GetLevelResource()
    {
        check(IsValid());
        if (UStructProperty* Prop = Cast<UStructProperty>(InnerStruct->GetStructPropertyByName("MapResource")))
        {
            if (Prop->Struct == FSoftLevelRef::StaticStruct())
            {
                void* PropData = Prop->ContainerPtrToValuePtr<void>(InnerStruct->GetStructMemory());
                if (FSoftLevelRef* SoftLevel = (FSoftLevelRef*)(PropData))
                {
                    return SoftLevel->LevelRef;
                }
            }
        }

        return nullptr;
    }

    FString GetStageConfigFile() 
    {
        check(IsValid());
        if (UStructProperty* Prop = Cast<UStructProperty>(InnerStruct->GetStructPropertyByName(FName("StageConfig"))))
        {
            FStageConfig* StageConfig = static_cast<FStageConfig*>(Prop->ContainerPtrToValuePtr<void>(InnerStruct->GetData()->GetStructMemory()));
            return StageConfig->FilePath;
        }
        return FString();
    }

    bool IsValid()
    {
        return InnerStruct.IsValid() && InnerStruct->IsValid();
    }

private:
    FStageScopePtr InnerStruct;
};

class FStageSceneProxy : public IStageScene, public FNotifyHook
{
public:
    virtual ~FStageSceneProxy() {}

    virtual void LoadStage(FStageScopePtr NewStage) override
    {
        if (StageProxy.GetStage() == NewStage)
        {
            return ;
        }

        StageProxy.InitStage(NewStage);
        FSimpleDelegate OnLoaded;
        OnLoaded.BindLambda([this]() {
            GetStageEditorInterface()->OnStageMapChanged();
        });

        //Load Level
        LevelLoader.AsyncLoadLevel(GetWorld(), StageProxy.GetLevelResource(), OnLoaded, true);

        //Load Config
        ImportStageObjConfigs();
    }

    virtual void ReLoadStage() override
    {
        CleanStage();
        ImportStageObjConfigs();
    }

    virtual void CleanStage(bool bFullClean = false) override
    {
        if (bFullClean)
        {
            // Clean Level Resource
            // TODO: 

            // Clean Stage Configed Objects
            CleanStageObjConfigs();
            StageProxy.Clean();
        }
        else
        {
            CleanStageObjConfigs();
        }
    }

    virtual FNotifyHook* GetStageSettingChangedHook() override
    {
        return this;
    }

    virtual FStageScopePtr GetCurrentStage() override
    {
        return StageProxy.GetStage();
    }

    UWorld* GetWorld()
    {
        FWorldContext* PIEWorldContext = GEditor->GetPIEWorldContext();
        UWorld* World = PIEWorldContext ? PIEWorldContext->World() : GEditor->GetEditorWorldContext().World();
        check(World);
        return World;
    }

    IStageEditorInterface* GetStageEditorInterface() { 
        FStageEditorModule& StageEd = FModuleManager::GetModuleChecked<FStageEditorModule>("StageEditor");
        return StageEd.GetProxyInterface();
    }

    /**
     * Stage 属性更改时触发
     * 本方法会触发场景刷新
     */
    virtual void NotifyPostChange( const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged ) override
    {
        FNotifyHook::NotifyPostChange(PropertyChangedEvent, PropertyThatChanged);
    }

    virtual void ImportStageObjConfigs()
    {
        // 1. read config file
        FString StageConfigFile = StageProxy.GetStageConfigFile();
        FString StageDataFilePath = GetDefault<UStageEditorSettings>()->StageDataFilePath.FilePath;
        FString StageConfigFilePath = FPaths::Combine(FPaths::GetPath(StageDataFilePath), StageProxy.GetStageConfigFile());

        // 2. import stage object configs
        FString FileString;
        if (FFileHelper::LoadFileToString(FileString, *StageConfigFilePath))
        {
            ImportSOCString(FileString);
        }
    }

    virtual void ExportStageObjConfigs() override
    {
        FString StageConfigFile = StageProxy.GetStageConfigFile();
        FString StageDataFilePath = GetDefault<UStageEditorSettings>()->StageDataFilePath.FilePath;
        ExportStageObjConfigsAs(FPaths::Combine(FPaths::GetPath(StageDataFilePath), StageProxy.GetStageConfigFile()));
    }

    virtual void ExportStageObjConfigsAs(const FString& FilePath) override
    {
        // 1. get stage object configs string
        FString OutString;
        ExportSOCString(OutString);
        // 2. save to file
        if (!FFileHelper::SaveStringToFile(OutString, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
        {
            UE_LOG(LogStageEditor, Error, TEXT("Error to save SOC data on path: %s"), *FilePath);
        }
    }

    void CleanStageObjConfigs()
    {
        UWorld* World = GetWorld();
        ULevel* PersistentLevel = World->PersistentLevel;
        for (AActor* Actor : PersistentLevel->Actors)
        {
            if (Actor != nullptr && Actor->GetClass()->ImplementsInterface(UStagePlacement::StaticClass()))
            {
                Actor->Destroy();
                World->RemoveActor(Actor, true);
            }
        }
        PersistentLevel->Modify();
        PersistentLevel->Model->Modify();
    }

private:
    // SOC : Stage Object configs
    void ExportSOCString(FString& OutString)
    {
        TMap<EPlacementIdentifier, TArray<FStageArchive>> PlacementArchives;
        const static UEnum* EnumPtr = StaticEnum<EPlacementIdentifier>();
        for (int i = 0; i < EnumPtr->NumEnums(); i++)
        {
            EPlacementIdentifier EnumValue = (EPlacementIdentifier)(EnumPtr->GetValueByIndex(i));
            PlacementArchives.Add(EnumValue, TArray<FStageArchive>());
        }

        TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>> > JsonWriter = 
            TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutString, 0);
        JsonWriter->WriteObjectStart();

        for(FActorIterator It(GetWorld()); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor && !Actor->IsPendingKill() 
                && Actor->GetClass()->ImplementsInterface(UStagePlacement::StaticClass()))
            {
                IStagePlacement* Placement = dynamic_cast<IStagePlacement*>(Actor);
                FStageArchive Archive;
                Archive.Transform = Actor->GetActorTransform();
                Placement->AssembleSerializeInfo(Archive);
                Archive.ObjClass = FSoftClassPath(Actor->GetClass());
                PlacementArchives[Archive.Identifier].Add(Archive);
            }
        }

        // Do json serialization
        for (TPair<EPlacementIdentifier, TArray<FStageArchive>> Pair : PlacementArchives)
        {
            JsonWriter->WriteArrayStart(EnumPtr->GetNameStringByIndex((int32)Pair.Key));
            for (const FStageArchive& Archive : Pair.Value)
            {
                TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

                // Transform Info
                {
                    TSharedRef<FJsonObject> TransformJson = MakeShared<FJsonObject>();
                    TransformToJsonObject(Archive.Transform, TransformJson);
                    JsonObject->SetObjectField(IDENTIFIER_TRANSFORM, TransformJson);
                }

                // Configs
                FJsonObjectConverter::UStructToJsonObject(
                    Archive.ScriptStruct.Get(),
                    const_cast<const void*>((void*)Archive.StructMemory),
                    JsonObject, 0, 0);
                JsonObject->SetStringField(IDENTIFIER_CLASS, Archive.ObjClass.ToString());

                // Class Info
                FJsonSerializer::Serialize(JsonObject, JsonWriter, false);
            }
            JsonWriter->WriteArrayEnd();
        }

        JsonWriter->WriteObjectEnd();
        JsonWriter->Close();
    }

    void ImportSOCString(const FString& InString)
    {
        UWorld* World = GetWorld();

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(InString);
        if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
        {
            UE_LOG(LogStageEditor, Warning, TEXT("ImportSOCString - Unable to parse SOC String =[%s]"), *InString);
            return;
        }

        TMap<EPlacementIdentifier, TArray< TSharedPtr<FJsonValue> > > PlacementArchives;
        const static UEnum* EnumPtr = StaticEnum<EPlacementIdentifier>();
        for (int i = 0; i < EnumPtr->NumEnums(); i++)
        {
            FString EnumName = EnumPtr->GetNameStringByIndex(i);
            EPlacementIdentifier EnumValue = (EPlacementIdentifier)(EnumPtr->GetValueByIndex(i));
            const TArray<TSharedPtr<FJsonValue>>* JsonArray;
            if (JsonObject->TryGetArrayField(EnumName, JsonArray))
            {
                PlacementArchives.Add(EnumValue, JsonObject->GetArrayField(EnumName));
            }
        }

        for (TPair<EPlacementIdentifier, TArray< TSharedPtr<FJsonValue> >> Pair : PlacementArchives)
        {
            for (TSharedPtr<FJsonValue> ArchiveJson : Pair.Value)
            {
                FStageArchive Archive;
                TSharedPtr<FJsonObject> ArchiveJsonObject = ArchiveJson->AsObject();
                // 1. Parse Class
                Archive.ObjClass = ArchiveJsonObject->GetStringField(IDENTIFIER_CLASS);
                UClass* PlacementClass = Archive.ObjClass.TryLoadClass<UClass>();
                if (PlacementClass == nullptr)
                {
                    PlacementClass = Cast<UClass>(Archive.ObjClass.TryLoad());
                }

                if ( PlacementClass )
                {
                    // 2. Parse Transform
                    TSharedPtr<FJsonObject> TransformJson = ArchiveJsonObject->GetObjectField(IDENTIFIER_TRANSFORM);
                    if (TransformJson.IsValid())
                    {
                        Archive.Transform = JsonObjectToTransform(TransformJson.ToSharedRef());
                    }

                    AActor* Actor = World->SpawnActor(PlacementClass, &Archive.Transform);

                    if (Actor->GetClass()->ImplementsInterface(UStagePlacement::StaticClass()))
                    {
                        IStagePlacement* Placement = dynamic_cast<IStagePlacement*>(Actor);
                        Placement->AssembleSerializeInfo(Archive);

                        // 3. Parse Config
                        if (!FJsonObjectConverter::JsonObjectToUStruct(ArchiveJsonObject.ToSharedRef(), Archive.ScriptStruct.Get(), Archive.StructMemory))
                        {
                            UE_LOG(LogStageEditor, Warning, TEXT("ImportSOCString - Unable to deserialize."));
                            continue;
                        }

                        Placement->PostSerialize();
                    }
                }
            }
        }
    }
private:
    /** Stage Struct */
    FStageStructProxy StageProxy;
    /** Level Loader */
    FLevelAsyncLoader LevelLoader;
};

IStageScene* IStageScene::Get()
{
    static IStageScene* sStageProxy = new FStageSceneProxy();
    return sStageProxy;
}
