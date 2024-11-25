// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GoogleSheetsApi.h"
#include "GoogleSheetsBridgeSettings.h"
#include "GoogleSheetsBridgeLogChannels.h"
#include "GSBAsset.h"

FGoogleSheetsApiParams_GET::FGoogleSheetsApiParams_GET(TSharedRef<const FGSBAsset> InAsset)
	: Asset(InAsset)
{
}

FString FGoogleSheetsApiParams_GET::GetUrl() const
{
	const UGoogleSheetsBridgeSettings* Settings = GetDefault<UGoogleSheetsBridgeSettings>();
	return FString::Printf(TEXT("https://script.google.com/macros/s/%s/exec?spreadsheetID=%s&sheetName=%s&assetPath=%s"),
		*Settings->ApiScriptId, *Asset->GetSpreadsheetId(), *Asset->GetFName().ToString(), *FSoftObjectPath(Asset->GetHandle()).ToString());
}

FGoogleSheetsApiParams_POST::FGoogleSheetsApiParams_POST(TSharedRef<const FGSBAsset> InAsset)
	: FGoogleSheetsApiParams_GET(InAsset)
{
}

void UGoogleSheetsApi::SendGetRequest(const FGoogleSheetsApiParams_GET& Params)
{
}

void UGoogleSheetsApi::SendPostRequest(const FGoogleSheetsApiParams_POST& Params)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	Request->SetVerb("POST");
	Request->SetURL(Params.GetUrl());
	Request->SetContentAsString(Params.Content);
	
	Request->SetHeader(TEXT("Content-Type"), TEXT("text/csv; charset=utf-8"));
	Request->SetHeader(TEXT("Accepts"), TEXT("text/plain"));
	
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::HandleResponseReceived_POST);
	Request->ProcessRequest();
}

void UGoogleSheetsApi::HandleResponseReceived_GET(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!IsResponseValid(Response, bWasSuccessful))
	{
		return;
	}

	OnResponseReceived_GET.Broadcast(Response->GetContentAsString());
}

void UGoogleSheetsApi::HandleResponseReceived_POST(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!IsResponseValid(Response, bWasSuccessful))
	{
		return;
	}

	OnResponseReceived_POST.Broadcast(Response->GetContentAsString());
}

bool UGoogleSheetsApi::IsResponseValid(FHttpResponsePtr Response, bool bWasSuccessful)
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
