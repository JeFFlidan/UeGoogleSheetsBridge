// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GoogleSheetsBridgeSettings.generated.h"

/**
 * 
 */
UCLASS(Config = GoogleSheetsBridgeSettings, DefaultConfig)
class GOOGLESHEETSBRIDGE_API UGoogleSheetsBridgeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, BlueprintReadOnly, Category = "General", meta = (ClampMin = 1, ClampMax = 65535))
	int32 ServerPort;

	UGoogleSheetsBridgeSettings(const FObjectInitializer& Initializer);
};
