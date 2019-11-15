#include "MovieSceneStateSection.h"
#include "SequencerChannelInterface.h"

// template<typename ChannelType>
struct TStateSequencerChannelInterface : public TSequencerChannelInterface<FMovieSceneStateSectionData>
{
public:
	virtual void ExtendKeyMenu_Raw(FMenuBuilder& MenuBuilder, TArrayView<const FExtendKeyMenuParams> ChannelsAndHandles, TWeakPtr<ISequencer> InSequencer) const override;
};


void ExtendKeyMenu(FMenuBuilder& OuterMenuBuilder, TArray<TExtendKeyMenuParams<FMovieSceneStateSectionData>>&& Channels, TWeakPtr<ISequencer> InSequencer)
{

}
