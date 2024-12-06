// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBPendingNotification.h"
#include "GSBUtils.h"

#include "Widgets/Notifications/SNotificationList.h"

void FGSBPendingNotification::ShowPendingNotification()
{
	Notification = FGSBUtils::ShowNotification_Pending(GetMessageFromTemplate(PendingMessageTemplate)); 
}

void FGSBPendingNotification::ShowSuccessNotification() const
{
	if (Notification.IsValid())
	{
		FGSBUtils::CloseNotification_Pending(
			Notification, GetMessageFromTemplate(ResultMessageTemplate + " was successful."), SNotificationItem::CS_Success);
	}
}

void FGSBPendingNotification::ShowFailNotification() const
{
	if (Notification.IsValid())
	{
		FGSBUtils::CloseNotification_Pending(
			Notification,
			GetMessageFromTemplate(ResultMessageTemplate + " was failed."), SNotificationItem::CS_Fail);
	}
}

FString FGSBPendingNotification::GetMessageFromTemplate(const FString& MessageTemplate) const
{
	return MessageTemplate.Replace(TEXT("%s"), *AssetName);
}
