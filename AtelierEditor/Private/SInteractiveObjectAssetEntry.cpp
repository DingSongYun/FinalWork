#include "SInteractiveObjectAssetEntry.h"
#include "SlateTypes.h"
#include "EditorStyle.h"
#include "SSpinBox.h"

void SInteractiveObjectAssetEntry::Construct(const FArguments& InArgs, const TSharedPtr<FInteractiveObject>& InItem)
{
	bIsPressed = false;

	RemovePanelVisible = EVisibility::Hidden;

	Item = InItem;
	const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle<FButtonStyle>("PlacementBrowser.Asset");
	NormalImage = &ButtonStyle.Normal;
	HoverImage = &ButtonStyle.Hovered;
	PressedImage = &ButtonStyle.Pressed;
	ChildSlot
		[
			SNew(SBorder)
			.Cursor(EMouseCursor::Hand)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				.Padding(2, 0, 4, 0)
				.FillWidth(1.0f)
				[			
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(0, 0, 0, 0)
						.AutoWidth()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.Padding(1, 0, 0, 0)
							.AutoWidth()
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
								.Text(FText::FromString(UTF8_TO_TCHAR("npcID:")))
							]

							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.Padding(2, 0, 0, 0)
							.AutoWidth()
							[
								SNew(SEditableText)
								.Visibility(EVisibility::Visible)
								.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
								.SelectAllTextWhenFocused(true)
								.Text(FText::FromString(FString::FromInt(Item->npcID)))
								.OnIsTypedCharValid(this, &SInteractiveObjectAssetEntry::IsCharacterValid)
								.OnTextCommitted(this,&SInteractiveObjectAssetEntry::SetNpcIDTextValue)
								.MinDesiredWidth(50)
								.VirtualKeyboardType(EKeyboardType::Keyboard_Number)
								.VirtualKeyboardTrigger(EVirtualKeyboardTrigger::OnAllFocusEvents)
								.ColorAndOpacity(FLinearColor(0.f, 1.f, 0.f, 1.0f))
							]
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(5, 0, 0, 0)
						.AutoWidth()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.Padding(1, 0, 0, 0)
							.AutoWidth()
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
								.Text(FText::FromString(UTF8_TO_TCHAR("交互ID：")))
							]

							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.Padding(2, 0, 0, 0)
							.AutoWidth()
							[
								SNew(SEditableText)
								.Visibility(EVisibility::Visible)
								.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
								.SelectAllTextWhenFocused(true)
								.Text(FText::FromString(FString::FromInt(Item->interactionID)))
								.OnIsTypedCharValid(this, &SInteractiveObjectAssetEntry::IsCharacterValid)
								.OnTextCommitted(this, &SInteractiveObjectAssetEntry::SetInteractionIDTextValue)
								.MinDesiredWidth(50)
								.VirtualKeyboardType(EKeyboardType::Keyboard_Number)
								.VirtualKeyboardTrigger(EVirtualKeyboardTrigger::OnAllFocusEvents)
								.ColorAndOpacity(FLinearColor(0.f, 1.f, 0.f, 1.0f))

							]
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(5, 0, 0, 0)
						.AutoWidth()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.Padding(0, 0, 0, 0)
							.AutoWidth()
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Asset.Name")
								.Text(FText::FromString(UTF8_TO_TCHAR("name:")))
								.HighlightText(InArgs._HighlightText)
							]

							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.Padding(2, 0, 0, 0)
							.AutoWidth()
							[
								SNew(SEditableText)
								.Visibility(EVisibility::Visible)
								.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
								.SelectAllTextWhenFocused(true)
								.Text(Item->name)
								.OnTextCommitted(this, &SInteractiveObjectAssetEntry::SetNpcNameTextValue)
								.MinDesiredWidth(300)
								.VirtualKeyboardTrigger(EVirtualKeyboardTrigger::OnAllFocusEvents)
								.ColorAndOpacity(FLinearColor(0.0f, 1.f, 0.0f, 1.0f))
							]
						]
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				.Padding(100, 0, 4, 0)
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(0, 0, 0, 1)
					.AutoHeight()
					[
						RemoveDataPanel()
					]
				]
			]
		];
}



void SInteractiveObjectAssetEntry::SetNpcNameTextValue(const FText& InText, ETextCommit::Type)
{
	Item->name = InText;
}

void SInteractiveObjectAssetEntry::SetNpcIDTextValue(const FText& InText, ETextCommit::Type)
{
	FString s = CheckOnTextValid(InText);
	Item->npcID = FCString::Atoi(*s);
}

void SInteractiveObjectAssetEntry::SetInteractionIDTextValue(const FText& InText, ETextCommit::Type)
{
	FString s = CheckOnTextValid(InText);
	Item->interactionID = FCString::Atoi(*s);
}

FReply SInteractiveObjectAssetEntry::OnMouseButtonDown(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SelectItemEventDelegate.ExecuteIfBound(true, Item);
	}
	return FReply::Unhandled();
}

void SInteractiveObjectAssetEntry::OnMouseEnter(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent)
{
	this->RemovePanelVisible = EVisibility::Visible;
}

void SInteractiveObjectAssetEntry::OnMouseLeave(const FPointerEvent & MouseEvent)
{
	this->RemovePanelVisible = EVisibility::Hidden;
}

bool SInteractiveObjectAssetEntry::IsPressed() const
{
	return bIsPressed;
}

void SInteractiveObjectAssetEntry::SetSelectItemEventDelegate(FSelectInteractiveEvent SelectDelegate)
{
	SelectItemEventDelegate = SelectDelegate;
}

void SInteractiveObjectAssetEntry::SetRemoveDataDelegate(FRemoveInteractiveItemEvent RemoveDelegate)
{
	RemoveItemEventDelegate = RemoveDelegate;
}



const FSlateBrush * SInteractiveObjectAssetEntry::GetBorder() const
{
	if (IsPressed())
	{
		return PressedImage;
	}
	else if (IsHovered())
	{
		return HoverImage;
	}
	else
	{
		return NormalImage;
	}
}

TSharedRef<SWrapBox> SInteractiveObjectAssetEntry::RemoveDataPanel()
{
	TSharedRef<SWrapBox> RemovePanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));


	RemovePanel->AddSlot()
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SInteractiveObjectAssetEntry::OnRemoveDataClicked)
					.ContentPadding(FMargin(6, 2))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
					[
						SNew(SHorizontalBox)
						// Icon
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
							.Text(FEditorFontGlyphs::Trash)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text(FText::FromString(UTF8_TO_TCHAR("删除")))
						]
					]
				]
			]
		];
	return RemovePanel;
}

FReply SInteractiveObjectAssetEntry::OnRemoveDataClicked()
{
	RemoveItemEventDelegate.ExecuteIfBound(Item);
	return FReply::Handled();
}



//-------------------------------------------------------------------------------------一下为交互配置entry------------------------------------------------------------------------------------------------------



void SInteractionConfigObjectAssetEntry::Construct(const FArguments& InArgs, const TSharedPtr<const FInteractionConfigObject>& InItem)
{
	bIsPressed = false;

	RemovePanelVisible = EVisibility::Hidden;

	Item = InItem;
	const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle<FButtonStyle>("PlacementBrowser.Asset");
	NormalImage = &ButtonStyle.Normal;
	HoverImage = &ButtonStyle.Hovered;
	PressedImage = &ButtonStyle.Pressed;
	ChildSlot
		[
			SNew(SBorder)
			.Cursor(EMouseCursor::Hand)
			[
				SNew(SHorizontalBox)			
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)	
				.HAlign(HAlign_Left)
				.Padding(2, 0, 4, 0)
				.FillWidth(1.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(0, 0, 0, 1)
					.AutoWidth()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(1, 0, 0, 0)
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text(FText::FromString(UTF8_TO_TCHAR("交互ID：")))
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(1, 0, 0, 0)
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Asset.Name")
							.Text(FText::FromString(FString::FromInt(Item->interactionID)))
							.HighlightText(InArgs._HighlightText)
							.ColorAndOpacity(FLinearColor(0.f, 1.f, 0.f, 1.0f))
						]
					]

					+ SHorizontalBox::Slot()
					.Padding(15, 0, 0, 1)
					.AutoWidth()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(1, 0, 0, 0)
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text(FText::FromString(UTF8_TO_TCHAR("name：")))
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(1, 0, 0, 0)
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Asset.Name")
							.Text(Item->name)
							.HighlightText(InArgs._HighlightText)
							.ColorAndOpacity(FLinearColor(0.f, 1.f, 0.f, 1.0f))
						]
					]
					
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				.Padding(10, 0, 4, 0)
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(0, 0, 0, 1)
					.AutoHeight()
					[
						RemoveDataPanel()
					]
				]
			]
		];
}

FReply SInteractionConfigObjectAssetEntry::OnMouseButtonDown(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SelectItemEventDelegate.ExecuteIfBound(true, Item);
	}
	return FReply::Unhandled();
}

void SInteractionConfigObjectAssetEntry::OnMouseEnter(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent)
{
	this->RemovePanelVisible = EVisibility::Visible;
}

void SInteractionConfigObjectAssetEntry::OnMouseLeave(const FPointerEvent & MouseEvent)
{
	this->RemovePanelVisible = EVisibility::Hidden;
}

bool SInteractionConfigObjectAssetEntry::IsPressed() const
{
	return bIsPressed;
}

void SInteractionConfigObjectAssetEntry::SetSelectItemEventDelegate(FSelectInteractionConfigEvent SelectDelegate)
{
	SelectItemEventDelegate = SelectDelegate;
}

void SInteractionConfigObjectAssetEntry::SetRemoveDataDelegate(FRemoveInteractionConfigItemEvent RemoveDelegate)
{
	RemoveItemEventDelegate = RemoveDelegate;
}



const FSlateBrush * SInteractionConfigObjectAssetEntry::GetBorder() const
{
	if (IsPressed())
	{
		return PressedImage;
	}
	else if (IsHovered())
	{
		return HoverImage;
	}
	else
	{
		return NormalImage;
	}
}

TSharedRef<SWrapBox> SInteractionConfigObjectAssetEntry::RemoveDataPanel()
{
	TSharedRef<SWrapBox> RemovePanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));


	RemovePanel->AddSlot()
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SInteractionConfigObjectAssetEntry::OnRemoveDataClicked)
					.ContentPadding(FMargin(6, 2))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
					[
						SNew(SHorizontalBox)
						// Icon
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
							.Text(FEditorFontGlyphs::Trash)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text(FText::FromString(UTF8_TO_TCHAR("删除")))
						]
					]
				]
			]
		];
	return RemovePanel;
}

FReply SInteractionConfigObjectAssetEntry::OnRemoveDataClicked()
{
	RemoveItemEventDelegate.ExecuteIfBound(Item);
	return FReply::Handled();
}
