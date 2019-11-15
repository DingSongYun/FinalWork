// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-04
#pragma once

#include "UObject/LazyObjectPtr.h"
#include "StateSequenceObjectReference.generated.h"

UENUM()
enum class EStateSequenceObjectReferenceType : uint8
{
	/** The reference relates to the context actor */
	ContextActor,
	/** The reference relates to an actor outside of the context actor actor */
	//ExternalActor,
	/** The reference relates to a component */
	Component,
};


/**
 * 我们暂时只支持基于单Actor/蓝图的StateSequence
 * 所以binding基本都是指向Owner Actor所附属的Component
 */
USTRUCT()
struct STATESEQUENCE_API FStateSequenceObjectReference
{
	GENERATED_BODY()
public:
	FStateSequenceObjectReference() {}

	FStateSequenceObjectReference(UObject* InObject, UObject* InContext);

	/** 绑定对象是Component */
	void CreateForComponent(class UActorComponent* InComponent);

	/** 绑定对象是Actor */
	void CreateForActor(AActor* InActor, AActor* ContextActor);

	/**
	 * Resolve this reference from the specified source actor
	 *
	 * @return The object
	 */
	UObject* Resolve(AActor* SourceActor) const;

	/**
	 * Equality comparator
	 */
	friend bool operator==(const FStateSequenceObjectReference& A, const FStateSequenceObjectReference& B)
	{
		return A.ActorId == B.ActorId && A.PathToComp == B.PathToComp;
	}
private:
	UPROPERTY()
	EStateSequenceObjectReferenceType Type;

	UPROPERTY()
	FGuid ActorId;

	/** Path to the component from its owner actor */
	UPROPERTY()
	FString PathToComp;
};

USTRUCT()
struct STATESEQUENCE_API FStateSequenceObjectReferenceArray
{
	GENERATED_BODY()
public:
	TArray<FStateSequenceObjectReference> References;
};

USTRUCT()
struct STATESEQUENCE_API FStateSequenceObjectReferences
{
	GENERATED_BODY()
public:
	/**
	 * 是否存在到该ObjectId的绑定
	 * @return true 存在
	 */
	bool HasBinding(const FGuid& ObjectId) const;

	/**
	 * 删除到Object的绑定
	 * @param Object的GUID
	 */
	void RemoveBinding(const FGuid& ObjectId);

	/**
	 * 添加到Object的绑定
	 *
	 * @param ObjectId	Object关联的GUID
	 * @param InObject	绑定的Object
	 * @param InContext	InObject从属的Context (either a UWorld, or an AActor)
	 */
	void AddBinding(const FGuid& ObjectId, UObject* InObject, UObject* InContext);

	/**
	 * 获取到ObjectId的绑定
	 *
	 * @param ObjectId
	 * @param InContext
	 * @param OutObjects
	 */
	void ResolveBinding(const FGuid& ObjectId, UObject* InContext, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const;

	/**
	 * 获取根绑定
	 */
	FGuid GetRootBinding();

private:
	UPROPERTY()
	TMap<FGuid, FStateSequenceObjectReferenceArray> BindingMap;
};