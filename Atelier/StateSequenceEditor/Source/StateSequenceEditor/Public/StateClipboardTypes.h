// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-06
#pragma once

#if WITH_EDITOR

#include "Sections/MovieSceneStateSection.h"

//#include "Channels/MovieSceneFloatChannel.h"
//#include "Sections/MovieSceneEventSection.h"
//#include "Sections/MovieSceneActorReferenceSection.h"
#include "MovieSceneClipboard.h"

namespace MovieSceneClipboard
{
	template<> inline FName GetKeyTypeName<FStatePayload>()
	{
		return "StatePayload";
	}
}

#endif