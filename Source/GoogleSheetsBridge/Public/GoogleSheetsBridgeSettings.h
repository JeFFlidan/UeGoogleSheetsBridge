// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "GoogleSheetsBridgeSettings.generated.h"

/**
 * 
 */
UCLASS(Config = GoogleSheetsBridgeSettings, DefaultConfig, DisplayName = "Google Sheets Bridge")
class GOOGLESHEETSBRIDGE_API UGoogleSheetsBridgeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	FString DefaultSpreadsheetId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Advanced")
	FString ApiScriptId;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Experimental", meta = (ClampMin = 1, ClampMax = 65535))
	int32 ServerPort;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Experimental")
	bool bEnableServer;

	UGoogleSheetsBridgeSettings(const FObjectInitializer& Initializer);
};
