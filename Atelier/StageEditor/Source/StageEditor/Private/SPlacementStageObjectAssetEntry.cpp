#include "SPlacementStageObjectAssetEntry.h"
#include "PlacementCategory.h"
#include "ActorFactories/ActorFactory.h"
#include "AssetDragDropOp.h"
#include "SBoxPanel.h"
#include "EditorClassUtils.h"

void SPlacementStageObjectAssetEntry::Construct(const FArguments& InArgs, const TSharedPtr<const FPlacementItem>& InItem)
{
	bIsPressed = false;

	Item = InItem;

	TSharedPtr< SHorizontalBox > ActorType = SNew( SHorizontalBox );

	const bool bIsClass = Item->AssetData.GetClass() == UClass::StaticClass();
	const bool bIsActor = bIsClass ? CastChecked<UClass>(Item->AssetData.GetAsset())->IsChildOf(AActor::StaticClass()) : false;

	AActor* DefaultActor = nullptr;
	if (Item->Factory != nullptr)
	{
		DefaultActor = Item->Factory->GetDefaultActor(Item->AssetData);
	}
	else if (bIsActor)
	{
		DefaultActor = CastChecked<AActor>(CastChecked<UClass>(Item->AssetData.GetAsset())->ClassDefaultObject);
	}

	UClass* DocClass = nullptr;
	TSharedPtr<IToolTip> AssetEntryToolTip;
	if(DefaultActor != nullptr)
	{
		DocClass = DefaultActor->GetClass();
		AssetEntryToolTip = FEditorClassUtils::GetTooltip(DefaultActor->GetClass());
	}

	if (!AssetEntryToolTip.IsValid())
	{
		AssetEntryToolTip = FSlateApplicationBase::Get().MakeToolTip(Item->DisplayName);
	}
	
	const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle<FButtonStyle>( "PlacementBrowser.Asset" );

	NormalImage = &ButtonStyle.Normal;
	HoverImage = &ButtonStyle.Hovered;
	PressedImage = &ButtonStyle.Pressed; 

	ChildSlot
	[
		SNew( SBorder )
		.BorderImage( this, &SPlacementStageObjectAssetEntry::GetBorder )
		.Cursor( EMouseCursor::GrabHand )
		.ToolTip( AssetEntryToolTip )
		[
			SNew( SHorizontalBox )

			+ SHorizontalBox::Slot()
			.Padding( 0 )
			.AutoWidth()
			[
				// Drop shadow border
				SNew( SBorder )
				.Padding( 4 )
				.BorderImage( FEditorStyle::GetBrush( "ContentBrowser.ThumbnailShadow" ) )
				[
					SNew( SBox )
					.WidthOverride( 35 )
					.HeightOverride( 35 )
					//[
					//	SNew( SPlacementAssetThumbnail, Item->AssetData )
					//	.ClassThumbnailBrushOverride( Item->ClassThumbnailBrushOverride )
					//	.AlwaysUseGenericThumbnail( Item->bAlwaysUseGenericThumbnail )
					//	.AssetTypeColorOverride( Item->AssetTypeColorOverride )
					//]
				]
			]

			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(2, 0, 4, 0)
			[
				SNew( SVerticalBox )
				+SVerticalBox::Slot()
				.Padding(0, 0, 0, 1)
				.AutoHeight()
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "PlacementBrowser.Asset.Name" )
					.Text( Item->DisplayName )
					.HighlightText(InArgs._HighlightText)
				]
			]
		]
	];
}

const FSlateBrush* SPlacementStageObjectAssetEntry::GetBorder() const
{
	if ( IsPressed() )
	{
		return PressedImage;
	}
	else if ( IsHovered() )
	{
		return HoverImage;
	}
	else
	{
		return NormalImage;
	}
}

bool SPlacementStageObjectAssetEntry::IsPressed() const
{
	return bIsPressed;
}

FReply SPlacementStageObjectAssetEntry::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsPressed = false;

	if (FEditorDelegates::OnAssetDragStarted.IsBound())
	{
		TArray<FAssetData> DraggedAssetDatas;
		DraggedAssetDatas.Add( Item->AssetData );
		FEditorDelegates::OnAssetDragStarted.Broadcast( DraggedAssetDatas, Item->Factory );
		return FReply::Handled();
	}

	if( MouseEvent.IsMouseButtonDown( EKeys::LeftMouseButton ) )
	{
		return FReply::Handled().BeginDragDrop( FAssetDragDropOp::New( Item->AssetData, Item->Factory ) );
	}
	else
	{
		return FReply::Handled();
	}
}

FReply SPlacementStageObjectAssetEntry::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if ( MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton )
	{
		bIsPressed = true;

		return FReply::Handled().DetectDrag( SharedThis( this ), MouseEvent.GetEffectingButton() );
	}

	return FReply::Unhandled();
}

FReply SPlacementStageObjectAssetEntry::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if ( MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton )
	{
		bIsPressed = false;
	}

	return FReply::Unhandled();
}