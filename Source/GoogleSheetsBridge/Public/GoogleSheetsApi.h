// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Http.h"
#include "GoogleSheetsApi.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(OnResponse, FString);

/**
 * 
 */
UCLASS()
class GOOGLESHEETSBRIDGE_API UGoogleSheetsApi : public UObject
{
	GENERATED_BODY()

public:
	OnResponse OnResponseReceived;
	
	void SendGetRequest(FString SheetID);

private:
	static const FString ApiBaseUrl;
	FHttpModule* HttpModule;

	void HandleGetResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SetRequestHeaders(TSharedRef<IHttpRequest>& Request);
	bool IsResponseValid(FHttpResponsePtr Response, bool bWasSuccessful);
	TSharedRef<IHttpRequest> RequestWithRoute(FString Subroute);
};
