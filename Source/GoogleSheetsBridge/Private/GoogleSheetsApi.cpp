// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GoogleSheetsApi.h"

#include "GoogleSheetsBridgeSettings.h"
#include "GoogleSheetsBridgeLogChannels.h"
#include "GSBPendingNotification.h"
#include "GSBAsset.h"
#include "GSBUtils.h"

FGoogleSheetsApiParams_GET::FGoogleSheetsApiParams_GET(FGSBAsset InAsset)
	: Asset(InAsset)
{
}

FString FGoogleSheetsApiParams_GET::GetUrl() const
{
	const UGoogleSheetsBridgeSettings* Settings = GetDefault<UGoogleSheetsBridgeSettings>();
	return FString::Printf(TEXT("https://script.google.com/macros/s/%s/exec?spreadsheetID=%s&sheetName=%s&assetPath=%s"),
		*Settings->ApiScriptId, *Asset.FindOrAddSpreadsheetId(), *Asset.GetFName().ToString(), *FSoftObjectPath(Asset.GetHandle()).ToString());
}

FGoogleSheetsApiParams_POST::FGoogleSheetsApiParams_POST(FGSBAsset InAsset)
	: FGoogleSheetsApiParams_GET(InAsset)
{
}

void FGoogleSheetsApi::SendRequest_GET(const FGoogleSheetsApiParams_GET& Params, FOnResponse OnResponseReceived)
{
	if (!FGSBUtils::AreSettingsValid(Params.Asset))
	{
		return;
	}
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	Request->SetVerb("GET");
	Request->SetURL(Params.GetUrl());
	
	TSharedPtr<FGSBPendingNotification> Notification = MakeShared<FGSBPendingNotification>();

	if (bEnableNotifications)
	{
		UE_LOG(LogGoogleSheetsBridge, Display, TEXT("Create notification"))
		Notification->SetAssetName(Params.Asset.GetFName().ToString());
		Notification->SetPendingMessageTemplate(TEXT("Synchronizing %s with Google Spreadsheet"));
		Notification->SetResultMessageTemplate(TEXT("Synchronization with Google Spreadsheet for %s)"));

		Notification->ShowPendingNotification();
	}
	
	BindResponseDelegate(Request,OnResponseReceived, Notification);
	
	Request->ProcessRequest();
}

void FGoogleSheetsApi::SendRequest_POST(const FGoogleSheetsApiParams_POST& Params, FOnResponse OnResponseReceived)
{
	if (!FGSBUtils::AreSettingsValid(Params.Asset))
	{
		return;
	}
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	Request->SetVerb("POST");
	Request->SetURL(Params.GetUrl());
	Request->SetContentAsString(Params.Content);
	
	Request->SetHeader(TEXT("Content-Type"), TEXT("text/csv; charset=utf-8"));
	Request->SetHeader(TEXT("Accepts"), TEXT("text/plain"));

	TSharedPtr<FGSBPendingNotification> Notification = MakeShared<FGSBPendingNotification>();

	if (bEnableNotifications)
	{
		Notification->SetAssetName(Params.Asset.GetFName().ToString());
		Notification->SetPendingMessageTemplate(TEXT("Sending %s data to Google Spreadsheet."));
		Notification->SetResultMessageTemplate(TEXT("Sending %s data to Google Spreadsheet"));

		Notification->ShowPendingNotification();
	}
	
	BindResponseDelegate(Request,OnResponseReceived, Notification);
	
	Request->ProcessRequest();
}

bool FGoogleSheetsApi::IsResponseValid(FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Request is not successful"));
		return false;
	}

	if (!Response.IsValid())
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Response object is invalid"));
		return false;
	}

	// Need to approve bad request because Google App Script returns a bad request for all put requests with content,
	// but updates sheet correctly. Really strange bug.
	if (EHttpResponseCodes::IsOk(Response->GetResponseCode()) || EHttpResponseCodes::BadRequest == Response->GetResponseCode())
	{
		return true;
	}

	UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Http Response returned error code: %d. Error message: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
	return false;
}

void FGoogleSheetsApi::BindResponseDelegate(
	TSharedRef<IHttpRequest> Request,
	FOnResponse OnResponseReceived,
	TSharedPtr<FGSBPendingNotification> Notification)
{
	Request->OnProcessRequestComplete().BindLambda([OnResponseReceived, Notification]
		(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (!IsResponseValid(Response, bWasSuccessful))
		{
			Notification->ShowFailNotification();
			
			return;
		}

		Notification->ShowSuccessNotification();
		
		OnResponseReceived.Execute(Response->GetContentAsString());
	});
}
