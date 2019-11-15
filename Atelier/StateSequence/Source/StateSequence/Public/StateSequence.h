// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-30

#pragma once

#include "MovieSceneSequence.h"
#include "MovieScene.h"
#include "StateSequenceObjectReference.h"

#include "StateSequence.generated.h"

struct FMovieSceneStateSectionData;
/**
 * 其实Sequence系列类没有真正定义Sequence行为
 * 仅仅提供一些Sequence行为特征(是否可以Possessable，是否可以Animat)，
 * 以及管理Possessable的binding
 **/
UCLASS(BlueprintType, Blueprintable)
class STATESEQUENCE_API UStateSequence : public UMovieSceneSequence
{
public:
	GENERATED_BODY()

	UStateSequence(const FObjectInitializer& ObjectInitializer);

	/** Initialize this state sequence */
	virtual void Initialize();

	//~ Begin: UMovieSceneSequence interface
	virtual void BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context) override;
	virtual bool CanPossessObject(UObject& Object, UObject* InPlaybackContext) const override;
	virtual void LocateBoundObjects(const FGuid& ObjectId, UObject* Context, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const override;
	virtual UMovieScene* GetMovieScene() const override;
	virtual UObject* GetParentObject(UObject* Object) const override;
	virtual void UnbindPossessableObjects(const FGuid& ObjectId) override;

	virtual bool CanAnimateObject(UObject& InObject) const override;
	virtual void GatherExpiredObjects(const FMovieSceneObjectCache& InObjectCache, TArray<FGuid>& OutInvalidIDs) const override;

	//~ End: UMovieSceneSequence interface

	FGuid GetRootBinding();

	/**
	 * 绑定Actor到当前Sequence
	 */
	FGuid AssignActor(AActor* InActor);
	AActor* GetOutterActor() { return OutterActor; }

	const class UMovieSceneStateTrack* GetStateTrack() const;
	FORCEINLINE const FMovieSceneStateSectionData& GetStateData() const;

	void Notify(int32 index);

private:
	/** 核心动画类，每个Sequence都有一个MovieScene, 
	 * UE4估计是考虑Sequence子列可能会有不同的MovieScene的实现
	 * 所以并没有在上层父类限定此成员变量,我们这里也用这个就好
	 */
	UPROPERTY()
	UMovieScene* MovieScene;

	UPROPERTY(transient)
	AActor* OutterActor;

protected:
	/**
	 * Sequence处理binding的核心类 
	 * UE4对该binding并没有做模式上的限定，是个自由类
	 * Sequence一些方法是直接转发到改类进行处理
	 * 基本上需要提供以下三个方法
	 *		1. addBinding(createBinding)
	 *		2. resolveBinding
	 *		3. removeBinding
	 */
	UPROPERTY()
	FStateSequenceObjectReferences BindingReferences;
};
