// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-31
#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

class FStateSequenceEditorStyle
	: public FSlateStyleSet
{
public:
	FStateSequenceEditorStyle()
		: FSlateStyleSet("StateSequenceEditorStyle")
	{
		const FVector2D Icon16x16(16.0f, 16.0f);
		const FVector2D Icon24x24(24.0f, 24.0f);
		const FVector2D Icon48x48(48.0f, 48.0f);
		SetContentRoot(FPaths::ProjectPluginsDir() / TEXT("Atelier/StateSequenceEditor/Resources"));

		Set( "StateSequencer.Assign", new IMAGE_BRUSH( "Icon_StateSequence_Assign_48x", Icon48x48 ) );
		Set( "StateSequencer.Assign.Small", new IMAGE_BRUSH( "Icon_StateSequence_Assign_48x", Icon24x24 ) );

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	static TSharedRef<FStateSequenceEditorStyle> Get()
	{
		if (!Singleton.IsValid())
		{
			Singleton = MakeShareable(new FStateSequenceEditorStyle);
		}
		return Singleton.ToSharedRef();
	}
	
	~FStateSequenceEditorStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
private:
	static TSharedPtr<FStateSequenceEditorStyle> Singleton;
};