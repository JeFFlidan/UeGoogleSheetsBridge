// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"

namespace GSB::Constants
{
	const FString SyncWithSpreadsheetButtonName = "Sync with Google Spreadsheet";
	const FString ExportToSpreadsheetButtonName = "Export to Google Spreadsheet";
	const FString ChangeSpreadsheetIDButtonName = "Change Google Spreadsheet ID";

	const FString SyncWithSpreadsheetTooltip = "Updates the selected asset from Google Sheet that was previously connected to the asset by "
		"\"Export to Google Spreadsheet\".";

	const FString ExportToSpreadsheetTooltip = "Exports Asset data to Google Spreadsheet. If it is a first export to a Google Spreadsheet,"
		" a new Sheet will be created in the Spreadsheet. If Spreadsheet ID is not configured using \"Change Google Spreadsheet ID\", "
		"default ID from \"Google Sheets Bridge\" Project Settings will be used (property \"Default Spreadsheet Id\"). "
		"The Spreadsheet ID is stored in the asset metadata.";

	const FString ChangeSpreadsheetIDTooltip = "Configures new Spreadsheet ID that is stored in Asset metadata. This ID will be used"
		" by \"Export to Google Spreadsheet\".";
}