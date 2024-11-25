// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBAsset.h"
#include "GoogleSheetsBridgeServer.h"
#include "GoogleSheetsBridgeSettings.h"
#include "GoogleSheetsBridgeLogChannels.h"

#include "HttpServerModule.h"
#include "HttpServerResponse.h"
#include "HttpServerRequest.h"
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
		HttpRouter->BindRoute(FHttpPath(HttpPathPOST), EHttpServerRequestVerbs::VERB_OPTIONS,
			FHttpRequestHandler::CreateUObject(this, &ThisClass::RequestOPTIONS));

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
	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(TEXT("GoogleSheetsBridge GET"), TEXT("text/plain"));
	OnComplete(MoveTemp(response));
	return true;
}

bool UGoogleSheetsBridgeServer::RequestPOST(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	FUTF8ToTCHAR CSVTCHARData(reinterpret_cast<const ANSICHAR*>(Request.Body.GetData()), Request.Body.Num());
	FString CSVData{CSVTCHARData.Length(), CSVTCHARData.Get() };

	const FString* AssetPath = Request.QueryParams.Find(TEXT("assetPath"));
	if (!AssetPath)
	{
		SendResponse(Request, OnComplete, EHttpServerResponseCodes::BadRequest, TEXT("No assetPath query param"));
		return true;
	}

	FGSBAsset Asset(LoadObject<UObject>(nullptr, **AssetPath));
	if (!Asset.ImportFromCSVString(CSVData))
	{
		FString ErrorMessage = FString::Printf(TEXT("Failed to import asset %s from csv string %s"),
			*Asset.GetFName().ToString(), *CSVData);
		
		SendResponse(Request, OnComplete, EHttpServerResponseCodes::BadRequest, ErrorMessage);
		return true;
	}
	
	SendResponse(Request, OnComplete);
	
	return true;
}

bool UGoogleSheetsBridgeServer::RequestOPTIONS(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	SendResponse(Request, OnComplete);
	return true;
}

void UGoogleSheetsBridgeServer::SendResponse(
	const FHttpServerRequest& Request,
	const FHttpResultCallback& OnComplete,
	EHttpServerResponseCodes ResponseCode,
	const FString& ErrorMessage)
{
	if (ResponseCode == EHttpServerResponseCodes::Ok)
	{
		TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Ok();
		SetCORSHeaders(Request, *Response);
		OnComplete(MoveTemp(Response));
		return;
	}

	UE_LOG(LogGoogleSheetsBridge, Error, TEXT("%s"), *ErrorMessage);
	TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Error(ResponseCode, L"", ErrorMessage);
	SetCORSHeaders(Request, *Response);
	OnComplete(MoveTemp(Response));
}

void UGoogleSheetsBridgeServer::SetCORSHeaders(const FHttpServerRequest& Request, FHttpServerResponse& Response)
{
	const TArray<FString>* OriginArray = Request.Headers.Find("origin");
	FString Origin = OriginArray ? (*OriginArray)[0] : "*";
	UE_LOG(LogGoogleSheetsBridge, Display, TEXT("Origin: %s"), *Origin)
	
	Response.Headers.FindOrAdd(TEXT("Access-Control-Allow-Origin")).Add(Origin);
	Response.Headers.FindOrAdd(TEXT("Access-Control-Allow-Headers")).Add(TEXT("Content-Type"));
	Response.Headers.FindOrAdd(TEXT("Access-Control-Allow-Methods")).Add(TEXT("OPTIONS, POST, GET"));
}
