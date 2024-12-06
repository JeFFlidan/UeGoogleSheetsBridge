// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Templates/SharedPointerFwd.h"

class SNotificationItem;

class FGSBPendingNotification
{
public:
	void SetAssetName(const FString& InAssetName) { AssetName = InAssetName; }
	void SetPendingMessageTemplate(const FString& InMessageTemplate) { PendingMessageTemplate = InMessageTemplate; }
	void SetResultMessageTemplate(const FString& InMessageTemplate) { ResultMessageTemplate = InMessageTemplate; }

	void ShowPendingNotification();
	void ShowSuccessNotification() const;
	void ShowFailNotification() const;

private:
	FString AssetName;
	FString PendingMessageTemplate;
	FString ResultMessageTemplate;
	TSharedPtr<SNotificationItem> Notification;

	FString GetMessageFromTemplate(const FString& MessageTemplate) const;
};