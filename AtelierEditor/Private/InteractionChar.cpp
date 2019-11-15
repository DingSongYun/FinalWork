// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: I-RUNG.YU
// Date: 2019-01-24

#include "InteractionChar.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "EngineGlobals.h"
#include "Components/SkeletalMeshComponent.h"
#include "COmponents/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"
#include "ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "EditorViewportClient.h"
#include "LevelEditorViewport.h"
#include "UnrealEdGlobals.h"
#include "EngineUtils.h"
#include "EditorUtilities.h"
#include "EditorLevelUtils.h"
#include "Settings/LevelEditorMiscSettings.h"
#include "EditorModeManager.h"


// Sets default values
AInteractionChar::AInteractionChar()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetVisibility(false);

	SkeletalBodyComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalBody"));
	SkeletalBodyComponent->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	SkeletalBodyComponent->SetSkeletalMesh(PreviewMesh);
	SkeletalBodyComponent->SetRelativeLocation(FVector(0, 0, -87));
}

AInteractionChar::~AInteractionChar()
{

}

#if WITH_EDITOR
void AInteractionChar::PostEditChangeChainProperty(FPropertyChangedChainEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	static const FName SOCKETS = "Sockets";
	static const FName PREVIEW_MESH = "PreviewMesh";

	if (PropertyChangedEvent.Property->GetFName() == PREVIEW_MESH)
	{
		SkeletalBodyComponent->SetSkeletalMesh(PreviewMesh);
	}
	if (PropertyChangedEvent.Property != nullptr)
	{
		if (PropertyChangedEvent.Property->GetFName() == SOCKETS)
		{
			UWorld * currentWorld = GEditor->GetLevelViewportClients()[0]->GetWorld();
			for (TActorIterator<AActor> ActorItr(currentWorld); ActorItr; ++ActorItr)
			{
				AInteractionPoint* EditorChar = Cast<AInteractionPoint>(*ActorItr);
				if (EditorChar) {
					AInteractionPoint* Socket = Cast<AInteractionPoint>(EditorChar);
					if (!chatData.Sockets.Contains(Socket)) {
						Socket->Destroy();
					}
				}
			}
		}
	}
}


void AInteractionChar::SpawnSocket(FSocketData data)
{
	UWorld * currentWorld = GEditor->GetLevelViewportClients()[0]->GetWorld();

	FVector objectPosition(0.f, 0.f, 0.0f);
	FRotator objectRotation(0.f, 0.f, 0.f); //in degrees
	FVector objectScale(1, 1, 1);
	FTransform objectTrasform(objectRotation, objectPosition, objectScale);
	ULevel * currentLevel = currentWorld->GetLevel(0);
	UClass * ActorClass = AInteractionPoint::StaticClass();
	AActor * newActorCreated = nullptr;
	newActorCreated = GEditor->AddActor(currentLevel, ActorClass, objectTrasform, true, RF_Public | RF_Standalone | RF_Transactional);
	AInteractionPoint* Socket = Cast<AInteractionPoint>(newActorCreated);
	Socket->Setup(data);

	GEditor->EditorUpdateComponents();
	currentWorld->UpdateWorldComponents(true, false);
	GLevelEditorModeTools().MapChangeNotify();

	RefreshSocketDataList();
}

void AInteractionChar::InitSocketList(TSharedPtr<const FInteractionConfigObject> item)
{
	for (int32 i = 0; i < item->socket.Num(); i++)
	{
		this->SpawnSocket(item->socket[i]);
	}
}

void AInteractionChar::RefreshSocketDataList()
{
	if (chatData.Sockets.Num() && chatData.SocketDatas.Num()) {
		chatData.Sockets.Empty();
		chatData.SocketDatas.Empty();
	}

	UWorld * currentWorld = GEditor->GetLevelViewportClients()[0]->GetWorld();
	for (TActorIterator<AActor> ActorItr(currentWorld); ActorItr; ++ActorItr)
	{
		AInteractionPoint* EditorChar = Cast<AInteractionPoint>(*ActorItr);
		if (EditorChar) {
			AInteractionPoint* Socket = Cast<AInteractionPoint>(EditorChar);
			chatData.Sockets.Add(Socket);
			FSocketData newdata = Socket->SocketData;
			newdata.transform = Socket->GetActorTransform();
			newdata.IconPath = Socket->GetIconPath();
			chatData.SocketDatas.Add(newdata);
		}
	}
}



void AInteractionChar::Setup(TSharedPtr<const FInteractionConfigObject> item)
{
	chatData.interactionID = item->interactionID;
	chatData.interactorDistance = item->interactorDistance;
	chatData.name = item->name.ToString();
	chatData.FilteredIndex = item->index;
	this->InitSocketList(item);

	for (int32 i = 0; i < item->status.Num(); i++) {
		FStatusData statusdata;
		
		UWorld * currentWorld = GEditor->GetLevelViewportClients()[0]->GetWorld();

		for (TActorIterator<AActor> ActorItr(currentWorld); ActorItr; ++ActorItr)
		{
			AInteractionPoint* Socket = Cast<AInteractionPoint>(*ActorItr);
			if (Socket) {
				if (item->status[i].mainSocketList.Contains(Socket->SocketData.id)) {
					statusdata.MainSockets.Add(Socket);
				}
				else if (item->status[i].SecondarySocketList.Contains(Socket->SocketData.id)) {
					statusdata.SubSockets.Add(Socket);
				}
			}
		}

		chatData.Status.Add(statusdata);
	}

	SetupModel(item->ModelName);
	Refresh();	
}

void AInteractionChar::SetupModel(FString ModelName)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	FARFilter MFilter;
	MFilter.ClassNames.Add(USkeletalMesh::StaticClass()->GetFName());
	MFilter.bRecursivePaths = true;
	TArray< FAssetData > MeshAssetList;
	AssetRegistry.GetAssets(MFilter, MeshAssetList);
	FString AssetPathContain = ModelName;
	for (FAssetData Asset : MeshAssetList)
	{
		if (Asset.PackageName.ToString().Contains(*AssetPathContain))
		{
			PreviewMesh = const_cast<USkeletalMesh*>(Cast<USkeletalMesh>(Asset.GetAsset()));
			SkeletalBodyComponent->SetSkeletalMesh(PreviewMesh);
			FHitResult hitres;
			SkeletalBodyComponent->K2_SetRelativeRotation(FRotator(0, -90, 0),true, hitres, true);
		}
	}
}

void AInteractionChar::RemoveAllSockets()
{
	if (chatData.Sockets.Num()) {
		for (auto& socket : chatData.Sockets) {
			socket->Destroy();
		}
	}
}


TSharedRef<FInteractionConfigObject> AInteractionChar::GetData()
{
	RefreshSocketDataList();
	FString MeshName;
	if (PreviewMesh != nullptr) {
		MeshName = FName(*PreviewMesh->GetName()).ToString();
	}
	TSharedRef<FInteractionConfigObject> it = MakeShareable(new FInteractionConfigObject());
	it->interactionID = chatData.interactionID;
	it->interactorDistance = chatData.interactorDistance;
	it->name = FText::FromString(chatData.name);
	it->ModelName = MeshName;
	it->socket = chatData.SocketDatas;

	for (int32 i = 0; i < chatData.Status.Num(); i++) {
		FStateData statedata;
		for (AInteractionPoint *mainsocket : chatData.Status[i].MainSockets) {
			statedata.mainSocketList.Add(mainsocket->SocketData.id);
		}
		for (AInteractionPoint *subsocket : chatData.Status[i].SubSockets) {
			statedata.SecondarySocketList.Add(subsocket->SocketData.id);
		}
		it->status.Add(statedata);
	}

	return it;
}
#endif

// Called when the game starts or when spawned
void AInteractionChar::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AInteractionChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractionChar::Refresh()
{	
#if WITH_EDITOR
	SetActorLabel(UTF8_TO_TCHAR("交互物_") + chatData.name);

	for (int32 i = 0; i < chatData.Sockets.Num(); i++)
	{
		chatData.Sockets[i]->Refresh();
	}
#endif
}

namespace slua {
	DefEnumClass(EInteractionAnimType, EInteractionAnimType::StateAnimIndex, EInteractionAnimType::AnimID);
	DefEnumClass(EInteractionAnimPlayerType, EInteractionAnimPlayerType::Interactive, EInteractionAnimPlayerType::Character);
}