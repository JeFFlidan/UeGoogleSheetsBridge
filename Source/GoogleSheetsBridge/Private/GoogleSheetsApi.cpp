// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GoogleSheetsApi.h"
#include "GoogleSheetsBridgeLogChannels.h"

const FString UGoogleSheetsApi::ApiBaseUrl = "https://docs.google.com/spreadsheets/d/";

void UGoogleSheetsApi::SendGetRequest(FString SheetID)
{
	TSharedRef<IHttpRequest> Request = RequestWithRoute(FString::Printf(TEXT("%s/export?format=csv"), *SheetID));
	Request->SetVerb("GET");
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::HandleGetResponseReceived);
	Request->ProcessRequest();
}

void UGoogleSheetsApi::HandleGetResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!IsResponseValid(Response, bWasSuccessful))
	{
		return;
	}

	OnResponseReceived.Broadcast(Response->GetContentAsString());
}

void UGoogleSheetsApi::SetRequestHeaders(TSharedRef<IHttpRequest>& Request)
{
	Request->SetHeader("User-Agent", "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", "text/csv");
	Request->SetHeader("Accepts", "text/csv");
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

	if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		return true;
	}

	UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Http Response returned error code: %d"), Response->GetResponseCode());
	return false;
}

TSharedRef<IHttpRequest> UGoogleSheetsApi::RequestWithRoute(FString Subroute)
{
	HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = HttpModule->CreateRequest();
	Request->SetURL(ApiBaseUrl + Subroute);
	SetRequestHeaders(Request);
	return Request;
}
