// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GoogleSheetsBridgeServer.h"
#include "GoogleSheetsBridgeSettings.h"
#include "GoogleSheetsBridgeLogChannels.h"

#include "HttpServerModule.h"
#include "HttpServerResponse.h"
#include "IHttpRouter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GoogleSheetsBridgeServer)

UGoogleSheetsBridgeServer::UGoogleSheetsBridgeServer()
{
	bIsServerStarted = false;
	HttpPathGET = TEXT("/get");
	HttpPathPOST = TEXT("/post");
	Settings = GetDefault<UGoogleSheetsBridgeSettings>();
}

void UGoogleSheetsBridgeServer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	StartServer();
}

void UGoogleSheetsBridgeServer::Deinitialize()
{
	StopServer();
	Super::Deinitialize();
}

void UGoogleSheetsBridgeServer::StartServer()
{
	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	TSharedPtr<IHttpRouter> HttpRouter = HttpServerModule.GetHttpRouter(Settings->ServerPort);

	if (HttpRouter.IsValid())
	{
		HttpRouter->BindRoute(FHttpPath(HttpPathGET), EHttpServerRequestVerbs::VERB_GET,
			FHttpRequestHandler::CreateUObject(this, &ThisClass::RequestGET));
		HttpRouter->BindRoute(FHttpPath(HttpPathPOST), EHttpServerRequestVerbs::VERB_POST,
			FHttpRequestHandler::CreateUObject(this, &ThisClass::RequestPOST));

		HttpServerModule.StartAllListeners();
		bIsServerStarted = true;

		UE_LOG(LogGoogleSheetsBridge, Display, TEXT("Google Sheets Bridge server is started on port = %d"), Settings->ServerPort);
	}
	else
	{
		UE_LOG(LogGoogleSheetsBridge, Display, TEXT("Failed to start Google Sheets Bridge server on port = %d"), Settings->ServerPort);
	}
}

void UGoogleSheetsBridgeServer::StopServer()
{
	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	HttpServerModule.StopAllListeners();
}

bool UGoogleSheetsBridgeServer::RequestGET(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	UE_LOG(LogGoogleSheetsBridge, Display, TEXT("GET request is successful."));
	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(TEXT("GoogleSheetsBridge GET"), TEXT("text/html"));
	OnComplete(MoveTemp(response));
	return true;
}

bool UGoogleSheetsBridgeServer::RequestPOST(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	UE_LOG(LogGoogleSheetsBridge, Display, TEXT("POST request is successful."));
	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(TEXT("GoogleSheetsBridge GET"), TEXT("text/html"));
	OnComplete(MoveTemp(response));
	return true;
}
