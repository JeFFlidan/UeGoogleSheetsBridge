// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Http.h"
#include "GoogleSheetsApi.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnResponse, FString);

struct FGoogleSheetsApiParams_GET
{
	FString SpreadsheetID;
	FName SheetName;
	
	FGoogleSheetsApiParams_GET(
		const FString& InSpreadsheetID,
		FName InSheetName);

	FString GetUrl() const;
};

struct FGoogleSheetsApiParams_POST : FGoogleSheetsApiParams_GET
{
	FString Content;
	
	FGoogleSheetsApiParams_POST(
		const FString& InSpreadsheetID,
		FName InSheetName);
};

/**
 * 
 */
UCLASS()
class GOOGLESHEETSBRIDGE_API UGoogleSheetsApi : public UObject
{
	GENERATED_BODY()

public:
	FOnResponse OnResponseReceived_GET;
	FOnResponse OnResponseReceived_POST;

	void SendGetRequest(const FGoogleSheetsApiParams_GET& Params);
	void SendPostRequest(const FGoogleSheetsApiParams_POST& Params);

private:
	void HandleResponseReceived_GET(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleResponseReceived_POST(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	bool IsResponseValid(FHttpResponsePtr Response, bool bWasSuccessful);
};
