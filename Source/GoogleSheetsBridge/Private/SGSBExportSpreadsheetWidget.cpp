// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "SGSBExportSpreadsheetWidget.h"
#include "GSBUtils.h"

void SGSBExportSpreadsheetWidget::Construct(const FArguments& InArgs)
{
	TSharedPtr<SWindow> Window = InArgs._Window;
	FGSBAsset Asset = InArgs._Asset;

	ChildSlot
	[
		SNew(SVerticalBox)
			+ SVerticalBox::Slot().Padding(10).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString("Spreadsheet ID:"))
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center)
					[
						SNew(SEditableTextBox)
						.MinDesiredWidth(700)
						.Text(FText::FromString(Asset.FindSpreadsheetId()))
						.OnTextChanged_Lambda([this](const FText& Id) mutable
						{
							SpreadsheetID = Id.ToString();
						})
					]
			]
			+ SVerticalBox::Slot().Padding(10).HAlign(HAlign_Right).VAlign(VAlign_Center)
			[
				SNew(SButton)
				.Text(FText::FromString("Change"))
				.OnClicked_Lambda([this, Asset, Window]() mutable ->FReply
				{
					Asset.SetSpreadsheetId(SpreadsheetID);
					if (Window.IsValid())
					{
						Window->RequestDestroyWindow();
					}
					return FReply::Handled();
				})
			]
	];
}
