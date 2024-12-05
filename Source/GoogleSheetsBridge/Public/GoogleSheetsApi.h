// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBAsset.h"
#include "Http.h"

DECLARE_DELEGATE_OneParam(FOnResponse, FString);

struct FGoogleSheetsApiParams_GET
{
	FGSBAsset Asset;
	
	FGoogleSheetsApiParams_GET(FGSBAsset InAsset);

	FString GetUrl() const;
};

struct FGoogleSheetsApiParams_POST : FGoogleSheetsApiParams_GET
{
	FString Content;
	
	FGoogleSheetsApiParams_POST(FGSBAsset InAsset);
};

class GOOGLESHEETSBRIDGE_API FGoogleSheetsApi
{
public:
	static void SendRequest_GET(const FGoogleSheetsApiParams_GET& Params, FOnResponse OnResponseReceived);
	static void SendRequest_POST(const FGoogleSheetsApiParams_POST& Params, FOnResponse OnResponseReceived);

private:
	static bool IsResponseValid(FHttpResponsePtr Response, bool bWasSuccessful);
	static void BindResponseDelegate(TSharedRef<IHttpRequest> Request, FOnResponse OnResponseReceived);
};
