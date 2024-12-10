// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBAsset.h"

#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"

class SWindow;

class FGSBUtils
{
public:
	static bool AssetToCsvString(const UCurveTable* Asset, FString& OutString);
	static bool AssetToCsvString(const UDataAsset* Asset, FString& OutString);
	static bool AssetToCsvString(const UDataTable* Asset, FString& OutString);

	static bool CSVStringToAsset(UCurveTable* Asset, const FString& CSVData);
	static bool CSVStringToAsset(UDataAsset* Asset, const FString& CSVData);
	static bool CSVStringToAsset(UDataTable* Asset, const FString& CSVData);

	static bool IsSyncButtonExecutable(FGSBAsset Asset)
	{
		return !Asset.FindSpreadsheetId().IsEmpty();
	}

	static bool AreSettingsValid(FGSBAsset Asset);

	static void GenericRequest_GET(FGSBAsset Asset, bool bEnableNotifications = true);
	static void GenericRequest_POST(FGSBAsset Asset, bool bEnableNotifications = true);
	static void CreateExportWindow(FGSBAsset Asset);

	static void ShowNotification_Success(
		const FString& Message,
		float ExpireDuration = 3.0f,
		float FadeInDuration = 0.5f,
		float FadeOutDuration = 0.5f);

	static void ShowNotification_Fail(
		const FString& Message,
		float ExpireDuration = 3.0f,
		float FadeInDuration = 0.5f,
		float FadeOutDuration = 0.5f);

	static TSharedPtr<SNotificationItem> ShowNotification_Pending(
		const FString& Message,
		float ExpireDuration = 3.0f,
		float FadeInDuration = 0.5f,
		float FadeOutDuration = 0.5f);

	static void CloseNotification_Pending(
		const TSharedPtr<SNotificationItem>& PendingNotification,
		const FString& NewMessage,
		SNotificationItem::ECompletionState NewCompletionState);

	static bool IsAssetLockedByCurrentUser(UObject* Asset, bool bShowNotifications);
	static bool IsSourceControlStatusUsed();
};
