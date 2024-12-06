// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBUtils.h"
#include "GSBConstants.h"
#include "GSBDataAssetCSV.h"
#include "GoogleSheetsApi.h"
#include "SGSBExportSpreadsheetWidget.h"
#include "GoogleSheetsBridgeLogChannels.h"
#include "GoogleSheetsBridgeSettings.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Framework/Notifications/NotificationManager.h"
#include "CurveTableEditorUtils.h"
#include "DataTableEditorUtils.h"
#include "Kismet/DataTableFunctionLibrary.h"

template<typename AssetType>
bool FGSBUtils::AssetToCsvString(const AssetType* Asset, FString& OutString)
{
	static_assert(!TIsSame<AssetType, AssetType>::Value, "Type is not supported");
	return false;
}

template<>
bool FGSBUtils::AssetToCsvString(const UCurveTable* Asset, FString& OutString)
{
	OutString = Asset->GetTableAsCSV();
	return !OutString.IsEmpty();
}

template<>
bool FGSBUtils::AssetToCsvString(const UDataAsset* Asset, FString& OutString)
{
	return FGSBDataAssetExporterCSV(OutString).WriteDataAsset(Asset);
}

template<>
bool FGSBUtils::AssetToCsvString(const UDataTable* Asset, FString& OutString)
{
	return UDataTableFunctionLibrary::ExportDataTableToCSVString(Asset, OutString);
}

template<typename AssetType>
bool FGSBUtils::CSVStringToAsset(AssetType* Asset, const FString& CSVData)
{
	static_assert(!TIsSame<AssetType, AssetType>::Value, "Type is not supported");
	return false;
}

template<>
bool FGSBUtils::CSVStringToAsset(UCurveTable* Asset, const FString& CSVData)
{
	TArray<FString> Errors = Asset->CreateTableFromCSVString(CSVData);

	if (!Errors.IsEmpty())
	{
		for (const FString& Error : Errors)
		{
			UE_LOG(LogGoogleSheetsBridge, Error, TEXT("%s"), *Error);
		}

		return false;
	}

	FCurveTableEditorUtils::BroadcastPostChange(Asset, FCurveTableEditorUtils::ECurveTableChangeInfo::RowList);

	return true;
}

template<>
bool FGSBUtils::CSVStringToAsset(UDataAsset* Asset, const FString& CSVData)
{
	return FGSBDataAssetImporterCSV(Asset, CSVData).ReadDataAsset();
}

template<>
bool FGSBUtils::CSVStringToAsset(UDataTable* Asset, const FString& CSVData)
{
	bool bResult = UDataTableFunctionLibrary::FillDataTableFromCSVString(Asset, CSVData);
	FDataTableEditorUtils::BroadcastPostChange(Asset, FDataTableEditorUtils::EDataTableChangeInfo::RowList);
	return bResult;
}

bool FGSBUtils::AreSettingsValid(FGSBAsset Asset)
{
	const UGoogleSheetsBridgeSettings* Settings = GetDefault<UGoogleSheetsBridgeSettings>();

	if (Settings->ApiScriptId.IsEmpty())
	{
		ShowNotification_Fail("No valid Google WebApp ID. Please, check Project Settings -> Google Sheets Bridge -> Api Script Id");
		return false;
	}

	if (Asset.FindSpreadsheetId().IsEmpty() && Settings->DefaultSpreadsheetId.IsEmpty())
	{
		FString Message = FString::Printf(TEXT("Asset %s does not have Spreadsheet ID and default ID is not configured. "
			"Please, check Project Settings -> Google Sheets Bridge -> Default Spreadsheet Id or configure custom ID using \"%s\""),
			*Asset.GetFName().ToString(), *GSB::Constants::ChangeSpreadsheetIDButtonName);
		
		ShowNotification_Fail(Message);
		return false;
	}

	return true;
}

void FGSBUtils::GenericRequest_GET(FGSBAsset Asset)
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

void FGSBUtils::GenericRequest_POST(FGSBAsset Asset)
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

void FGSBUtils::CreateExportWindow(FGSBAsset Asset)
{
	TSharedRef<SWindow> Window =
		SNew(SWindow).Title(FText::FromString(GSB::Constants::ChangeSpreadsheetIDButtonName))
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

TSharedPtr<SNotificationItem> CreateNotification(
	const FString& Message,
	SNotificationItem::ECompletionState CompletionState,
	float ExpireDuration,
	float FadeInDuration,
	float FadeOutDuration)
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.FadeInDuration = FadeInDuration;
	Info.FadeOutDuration = FadeOutDuration;
	Info.ExpireDuration = ExpireDuration;
	Info.bUseThrobber = CompletionState == SNotificationItem::CS_Pending;
	Info.bFireAndForget = CompletionState != SNotificationItem::CS_Pending;

	TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
	Notification->SetCompletionState(CompletionState);
	return Notification;
}

void FGSBUtils::ShowNotification_Success(
	const FString& Message,
	float ExpireDuration,
	float FadeInDuration,
	float FadeOutDuration)
{
	TSharedPtr<SNotificationItem> Notification = CreateNotification(
		Message, SNotificationItem::CS_Success, ExpireDuration, FadeInDuration, FadeOutDuration);

	Notification->ExpireAndFadeout();
}

void FGSBUtils::ShowNotification_Fail(
	const FString& Message,
	float ExpireDuration,
	float FadeInDuration,
	float FadeOutDuration)
{
	TSharedPtr<SNotificationItem> Notification = CreateNotification(
		Message, SNotificationItem::CS_Fail, ExpireDuration, FadeInDuration, FadeOutDuration);

	Notification->ExpireAndFadeout();
}

TSharedPtr<SNotificationItem> FGSBUtils::ShowNotification_Pending(
	const FString& Message,
	float ExpireDuration,
	float FadeInDuration,
	float FadeOutDuration)
{
	TSharedPtr<SNotificationItem> Notification = CreateNotification(
		Message, SNotificationItem::CS_Pending, ExpireDuration, FadeInDuration, FadeOutDuration);

	return Notification;
}

void FGSBUtils::CloseNotification_Pending(
	const TSharedPtr<SNotificationItem>& PendingNotification,
	const FString& NewMessage,
	SNotificationItem::ECompletionState NewCompletionState)
{
	if (!PendingNotification.IsValid())
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Notification is not valid"));
		return;
	}
	
	if (PendingNotification->GetCompletionState() != SNotificationItem::CS_Pending)
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Notification is not pending"));
		return;
	}

	PendingNotification->SetText(FText::FromString(NewMessage));
	PendingNotification->SetCompletionState(NewCompletionState);
	PendingNotification->ExpireAndFadeout();
}
