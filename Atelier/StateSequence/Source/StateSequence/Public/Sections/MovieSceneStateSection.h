// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-05
#pragma once

#include "CoreMinimal.h"

#include "MovieSceneSection.h"
#include "Channels/MovieSceneChannel.h"
#include "Channels/MovieSceneChannelData.h"
#include "Channels/MovieSceneChannelTraits.h"
#include "MultiBoxBuilder.h"
#include "MovieSceneStateNotify.h"
#include "MovieScene.h"
#include "MovieSceneStateSection.generated.h"

USTRUCT()
struct STATESEQUENCE_API FStatePayload
{
	GENERATED_BODY()
public:
	FStatePayload() {}
	FStatePayload(FName InStateName)
		:  StateName(InStateName)
	{}

	friend bool operator==(const FStatePayload& A, const FStatePayload& B)
	{
		return A.StateName == B.StateName;
	}

	friend bool operator!=(const FStatePayload& A, const FStatePayload& B)
	{
		return  A.StateName != B.StateName;
	}
	
	UPROPERTY(EditAnywhere, Category = State)
	FName StateName;

	UPROPERTY(EditAnywhere, Category = State)
	TArray<UMovieSceneStateNotify*> NotifyList;
};

USTRUCT()
struct STATESEQUENCE_API FMovieSceneStateSectionData : public FMovieSceneChannel
{
	GENERATED_BODY()
public:
	/**
	 * Access a mutable interface for this channel's data
	 *
	 * @return An object that is able to manipulate this channel's data
	 */
	FORCEINLINE TMovieSceneChannelData<FStatePayload> GetData()
	{
		return TMovieSceneChannelData<FStatePayload>(&Times, &Values, &KeyHandles);
	}

	/**
	 * Access a constant interface for this channel's data
	 *
	 * @return An object that is able to interrogate this channel's data
	 */
	FORCEINLINE TMovieSceneChannelData<const FStatePayload> GetData() const
	{
		return TMovieSceneChannelData<const FStatePayload>(&Times, &Values);
	}

	TArrayView<const FFrameNumber> GetKeyTimes() const
	{
		return Times;
	}

	TArrayView<const FStatePayload> GetKeyValues() const
	{
		return Values;
	}

	// ~ Begin: FMovieSceneChannel Interface
	virtual void GetKeys(const TRange<FFrameNumber>& WithinRange, TArray<FFrameNumber>* OutKeyTimes, TArray<FKeyHandle>* OutKeyHandles) override;
	virtual void GetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<FFrameNumber> OutKeyTimes) override;
	virtual void SetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<const FFrameNumber> InKeyTimes) override;
	virtual void DuplicateKeys(TArrayView<const FKeyHandle> InHandles, TArrayView<FKeyHandle> OutNewHandles) override;
	virtual void DeleteKeys(TArrayView<const FKeyHandle> InHandles) override;
	virtual void ChangeFrameResolution(FFrameRate SourceRate, FFrameRate DestinationRate) override;
	virtual TRange<FFrameNumber> ComputeEffectiveRange() const override;
	virtual int32 GetNumKeys() const override;
	virtual void Reset() override;
	virtual void Offset(FFrameNumber DeltaPosition) override;
	virtual void Optimize(const FKeyDataOptimizationParams& InParameters) override;
	virtual void ClearDefault() override;
	// ~ End: FMovieSceneChannel Interface

	/** Will Trigger when add new key */
	bool Evaluate(FFrameTime InTime, FStatePayload& OutValue) const;

	//执行当前State的所有Notify
	void NotifyAll(AActor* owner, int32 index) const;

	//根据State的Index获取Notify数据
	TArray<UMovieSceneStateNotify*> GetNotifyList(int32 index);

	//添加Notify到数据中
	void AddNotifyToData(int32 index, UClass* inclass);

	//从数据中移除相应的Notify
	void RemoveNotifyToData(int32 index, int32 offset);

	//从数据中替换相应的Notify
	void ReplaceNotifyToData(int32 index, int32 offset, UClass* inclass);

	//绘制Notify信息到State
	void DrawNotifyInfo(FMenuBuilder& MenuBuilder, FKeyHandle KeyHandle, UMovieScene* moviescene);

	//添加Notify按钮事件
	void AddNotify(FMenuBuilder& MenuBuilder, int32 index);

	//修改Notify按钮事件（包括删除和替换）
	void ModifyNotify(FMenuBuilder& MenuBuilder, int32 index, int32 offset);

	//绘制当前资源包中可以使用的Notify信息
	void MakeNewNotifyPicker(FMenuBuilder& MenuBuilder, bool bIsReplaceWithMenu, int32 index, int32 offset);

	//创建Notify蓝图类按钮事件
	void OnNewNotifyClicked();

	//创造新的Notify蓝图类
	void CreateNewNotify(const FText& NewNotifyName, ETextCommit::Type CommitInfo);

public:
	/** Key frames */
	UPROPERTY()
	TArray<FFrameNumber> Times;

	/** Data that correspond to each key time */
	UPROPERTY()
	TArray<FStatePayload> Values;

	FMovieSceneKeyHandleMap KeyHandles;

	UMovieScene* CurrentMovieScene;

	TWeakPtr<ISequencer> InSequencer;
};

template<>
struct TStructOpsTypeTraits<FMovieSceneStateSectionData> : public TStructOpsTypeTraitsBase2<FMovieSceneStateSectionData>
{
	enum { WithPostSerialize = false };
};

template<>
struct TMovieSceneChannelTraits<FMovieSceneStateSectionData> : TMovieSceneChannelTraitsBase<FMovieSceneStateSectionData>
{
	enum { SupportsDefaults = false };
};

/** Stub out unnecessary functions */
inline bool EvaluateChannel(const FMovieSceneStateSectionData* InChannel, FFrameTime InTime, FStatePayload& OutValue)
{
	// Can't evaluate event section data in the typical sense
	return InChannel->Evaluate(InTime, OutValue);
}

inline bool ValueExistsAtTime(const FMovieSceneStateSectionData* InChannel, FFrameNumber Time, const FStatePayload& Value)
{
	// true if any value exists
	return InChannel->GetData().FindKey(Time) != INDEX_NONE;
}


/**
 * State section
 */
UCLASS()
class STATESEQUENCE_API UMovieSceneStateSection
	: public UMovieSceneSection
{
	GENERATED_UCLASS_BODY()

public:
	const FMovieSceneStateSectionData& GetStateData() const
	{
		return StateData;
	}

private:
	UPROPERTY()
	FMovieSceneStateSectionData StateData;
};