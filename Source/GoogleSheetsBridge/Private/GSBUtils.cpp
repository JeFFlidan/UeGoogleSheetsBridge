// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBUtils.h"
#include "GSBConstants.h"
#include "GoogleSheetsApi.h"
#include "SGSBExportSpreadsheetWidget.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"

void GSB::GenericRequest_GET(FGSBAsset Asset)
{
	FGoogleSheetsApiParams_GET Params(Asset);

	FOnResponse OnResponse;
	OnResponse.BindLambda([Asset](FString Content) mutable
	{
		if (Content.IsEmpty())
		{
			return;
		}
				
		if (!Asset.ImportFromCSVString(Content))
		{
			UE_LOG(LogGoogleSheetsBridge, Display, TEXT("Failed to import %s from CSV %s"),
				*Asset.GetFName().ToString(), *Content);
		}
	});

	FGoogleSheetsApi::SendRequest_GET(Params, OnResponse);
}

void GSB::GenericRequest_POST(FGSBAsset Asset)
{
	FGoogleSheetsApiParams_POST Params(Asset);
	if (Asset.ExportToCSVString(Params.Content))
	{
		FOnResponse OnResponse;
		OnResponse.BindLambda([](FString Content)
		{
			// Because of bug with bad request response, does not work correctly for now
			UE_LOG(LogGoogleSheetsBridge, Verbose, TEXT("%s"), *Content);
		});
		FGoogleSheetsApi::SendRequest_POST(Params, OnResponse);
	}
	else
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("POST params are invalid."));
	}
}

void GSB::CreateExportWindow(FGSBAsset Asset)
{
	TSharedRef<SWindow> Window =
		SNew(SWindow).Title(FText::FromString(Constants::ExportToCustomButtonName))
		.ClientSize(FVector2D(500, 100))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	TSharedRef<SGSBExportSpreadsheetWidget> ExportWidget =
		SNew(SGSBExportSpreadsheetWidget)
		.Window(Window.ToSharedPtr())
		.Asset(Asset);

	Window->SetContent(ExportWidget);
	
	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));

	if (MainFrameModule.GetParentWindow().IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(Window, MainFrameModule.GetParentWindow().ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(Window);
	}
}
