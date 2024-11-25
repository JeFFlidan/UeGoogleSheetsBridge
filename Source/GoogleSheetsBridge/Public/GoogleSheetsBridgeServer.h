// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "EditorSubsystem.h"
#include "HttpServerRequest.h"
#include "HttpResultCallback.h"
#include "GoogleSheetsBridgeServer.generated.h"

class UGoogleSheetsBridgeSettings;

/**
 * 
 */
UCLASS()
class GOOGLESHEETSBRIDGE_API UGoogleSheetsBridgeServer : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UGoogleSheetsBridgeServer();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	bool bIsServerStarted;

	FString HttpPathGET;
	FString HttpPathPOST;

	UPROPERTY()
	TObjectPtr<const UGoogleSheetsBridgeSettings> Settings;
	
	void StartServer();
	void StopServer();

	bool RequestGET(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestPOST(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestOPTIONS(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	void SendResponse(
		const FHttpServerRequest& Request,
		const FHttpResultCallback& OnComplete,
		EHttpServerResponseCodes ResponseCode = EHttpServerResponseCodes::Ok,
		const FString& ErrorMessage = "Error");
	void SetCORSHeaders(const FHttpServerRequest& Request, FHttpServerResponse& Response);
};
