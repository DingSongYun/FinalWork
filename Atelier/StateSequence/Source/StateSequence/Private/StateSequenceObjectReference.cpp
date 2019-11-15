// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-04
#include "StateSequenceObjectReference.h"
#include "Engine/SCS_Node.h"
#include "Engine/Blueprint.h"
#include "UObject/Package.h"
#include "Components/ActorComponent.h"

FStateSequenceObjectReference::FStateSequenceObjectReference(UObject* InObject, UObject* InContext)
{
	if (UActorComponent* Component = Cast<UActorComponent>(InObject))
	{
		CreateForComponent(Component);
	}
	else if (AActor* Actor = Cast<AActor>(InObject))
	{
		AActor* ActorContext = CastChecked<AActor>(InContext);
		CreateForActor(Actor, ActorContext);
	}
	else
	{
		ensureMsgf(false, TEXT("Unsupport Object to bind."));
	}
}

void FStateSequenceObjectReference::CreateForComponent(class UActorComponent* InComponent)
{
	check(InComponent);
	Type = EStateSequenceObjectReferenceType::Component;

	if (AActor* Actor = InComponent->GetOwner())
	{
		PathToComp = InComponent->GetPathName(Actor);
		return;
	}

	UBlueprintGeneratedClass* GeneratedClass = InComponent->GetTypedOuter<UBlueprintGeneratedClass>();
	if (GeneratedClass && GeneratedClass->SimpleConstructionScript)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(GeneratedClass->SimpleConstructionScript))
		{
			for (USCS_Node* Node : GeneratedClass->SimpleConstructionScript->GetAllNodes())
			{
				if (Node->ComponentTemplate == InComponent)
				{
					PathToComp = Node->GetVariableName().ToString();
				}
			}
		}
	}


	ensureMsgf(false, TEXT("Can not find parent actor for component."));
}

void FStateSequenceObjectReference::CreateForActor(AActor* InActor, AActor* ContextActor)
{
	if (InActor != ContextActor)
	{
		ensureMsgf(false, TEXT("Can not to possessable external actor."));
		return;
	}

	Type = EStateSequenceObjectReferenceType::ContextActor;
}

UObject* FStateSequenceObjectReference::Resolve(AActor* SourceActor) const
{
	check(SourceActor);
	switch (Type)
	{
	case EStateSequenceObjectReferenceType::Component:
		check(!PathToComp.IsEmpty())

		return FindObject<UActorComponent>(SourceActor, *PathToComp);
		break;
	case EStateSequenceObjectReferenceType::ContextActor:
		return SourceActor;
		break;
	}

	return nullptr;
}

bool FStateSequenceObjectReferences::HasBinding(const FGuid& ObjectId) const
{
	return BindingMap.Contains(ObjectId);
}

void FStateSequenceObjectReferences::RemoveBinding(const FGuid& ObjectId)
{
	 BindingMap.Remove(ObjectId);
}

void FStateSequenceObjectReferences::AddBinding(const FGuid& ObjectId, UObject* InObject, UObject* InContext)
{
	BindingMap.FindOrAdd(ObjectId).References.Emplace(InObject, InContext);
}

void FStateSequenceObjectReferences::ResolveBinding(const FGuid& ObjectId, UObject* InContext, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const
{
	AActor* ContextActor = Cast<AActor>(InContext);
	if (ContextActor == nullptr)
	{
		// Invalid Context
		return;
	}

	if (const FStateSequenceObjectReferenceArray* ReferenceArray = BindingMap.Find(ObjectId))
	{
		for (const FStateSequenceObjectReference& Reference : ReferenceArray->References)
		{
			if (UObject* Object = Reference.Resolve(ContextActor))
			{
				OutObjects.Add(Object);
			}
		}
	}

}

FGuid FStateSequenceObjectReferences::GetRootBinding()
{
	TArray<FGuid> KeyArray;
	BindingMap.GenerateKeyArray(KeyArray);
	if (KeyArray.Num() > 0)
	{
		return KeyArray[0];
	}

	return FGuid();
}
