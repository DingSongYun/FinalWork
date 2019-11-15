#pragma once
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Input/Reply.h"
#include "SWrapBox.h"
#include "SEditableText.h"

class InteractionEntryBase
{
public:
	bool IsCharacterValid(TCHAR InChar) const
	{
		if (InChar > '9' || InChar < '0')
		{
			return false;
		}

		return true;
	}

	FString CheckOnTextValid(const FText& NewText)
	{
		FString s = NewText.ToString();
		TArray<TCHAR> chars;
		chars = s.GetCharArray();
		FString result ="";
		for (int i = 0; i < chars.Num(); i++)
		{
			if (IsCharacterValid(chars[i]))
			{
				result += chars[i];
			}
		}
		return result;
	}


private:

};


///---------------------------------------------------交互物entry---------------------------------------------------------------------

DECLARE_DELEGATE_TwoParams(FSelectInteractiveEvent, bool, TSharedPtr<FInteractiveObject>);
DECLARE_DELEGATE_OneParam(FRemoveInteractiveItemEvent, TSharedPtr<FInteractiveObject>);
class SInteractiveObjectAssetEntry : public SCompoundWidget , public InteractionEntryBase
{
public:
	SLATE_BEGIN_ARGS(SInteractiveObjectAssetEntry) {}

	/** Highlight this text in the text block */
	SLATE_ATTRIBUTE(FText, HighlightText)

	SLATE_END_ARGS()
		
	void Construct(const FArguments& InArgs, const TSharedPtr<FInteractiveObject>& InItem);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	bool IsPressed() const;

	TSharedPtr<FInteractiveObject> Item;

	void SetSelectItemEventDelegate(FSelectInteractiveEvent SelectDelegate);
	void SetRemoveDataDelegate(FRemoveInteractiveItemEvent RemoveDelegate);	
private:

	const FSlateBrush* GetBorder() const;

	bool bIsPressed;


	/** Brush resource that represents a button */
	const FSlateBrush* NormalImage;
	/** Brush resource that represents a button when it is hovered */
	const FSlateBrush* HoverImage;
	/** Brush resource that represents a button when it is pressed */
	const FSlateBrush* PressedImage;


	FSelectInteractiveEvent SelectItemEventDelegate;
	FRemoveInteractiveItemEvent RemoveItemEventDelegate;

	TSharedRef<SWrapBox> RemoveDataPanel();

	EVisibility RemovePanelVisible;

	FReply OnRemoveDataClicked();

	void SetNpcNameTextValue(const FText& InText, ETextCommit::Type);
	void SetNpcIDTextValue(const FText& InText, ETextCommit::Type);
	void SetInteractionIDTextValue(const FText& InText, ETextCommit::Type);
};

///---------------------------------------------------交互配置entry---------------------------------------------------------------------
DECLARE_DELEGATE_TwoParams(FSelectInteractionConfigEvent, bool, TSharedPtr<const FInteractionConfigObject>);
DECLARE_DELEGATE_OneParam(FRemoveInteractionConfigItemEvent, TSharedPtr<const FInteractionConfigObject>);
class SInteractionConfigObjectAssetEntry : public SCompoundWidget, public InteractionEntryBase
{
public:
	SLATE_BEGIN_ARGS(SInteractionConfigObjectAssetEntry) {}

	/** Highlight this text in the text block */
	SLATE_ATTRIBUTE(FText, HighlightText)

		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedPtr<const FInteractionConfigObject>& InItem);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	bool IsPressed() const;

	TSharedPtr<const FInteractionConfigObject> Item;

	void SetSelectItemEventDelegate(FSelectInteractionConfigEvent SelectDelegate);
	void SetRemoveDataDelegate(FRemoveInteractionConfigItemEvent RemoveDelegate);
private:

	const FSlateBrush* GetBorder() const;

	bool bIsPressed;



	/** Brush resource that represents a button */
	const FSlateBrush* NormalImage;
	/** Brush resource that represents a button when it is hovered */
	const FSlateBrush* HoverImage;
	/** Brush resource that represents a button when it is pressed */
	const FSlateBrush* PressedImage;


	FSelectInteractionConfigEvent SelectItemEventDelegate;
	FRemoveInteractionConfigItemEvent RemoveItemEventDelegate;

	TSharedRef<SWrapBox> RemoveDataPanel();

	EVisibility RemovePanelVisible;

	FReply OnRemoveDataClicked();

};