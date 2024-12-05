// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"

namespace GSB::Constants
{
	const FString SyncWithButtonName = "Sync with Google Sheet";
	const FString ExportToDefaultButtonName = "Export to Default Google Spreadsheet";
	const FString ExportToCustomButtonName = "Export to Custom Google Spreadsheet";

	const FString SyncWithTooltip = "Updates the selected asset from Google Sheet that was previously connected to the asset by "
		"\"Export to Default Google Spreadsheet\" or \"Export to Custom Google Spreadsheet\".";

	const FString ExportToDefaultTooltip = "Exports to Google Spreadsheet whose ID can be configured in \"Google Sheets Bridge\" "
		"Project Settings (property \"Default Spreadsheet Id\"). The Spreadsheet ID will be stored in the asset metadata, "
		"so you need to use this button once for the selected asset.";

	const FString ExportToCustomTooltip = "Exports to Google Spreadsheet whose ID is chosen by you before exporting. "
		"The Spreadsheet ID will be stored in the asset metadata. Each export by this button will override the Spreadsheet "
		"ID in the metadata, so in most cases you need to use it once for the selected asset.";
}