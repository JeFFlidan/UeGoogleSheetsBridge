// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBAsset.h"
#include "GoogleSheetsBridgeSettings.h"

#include "EditorAssetLibrary.h"

const FName METADATA_TAG_SPREADSHEET_ID = "GSB.SpreadsheetId";

void FGSBAssetBase::SetSpreadsheetId(const FString& SpreadsheetId)
{
	check(Asset);
	UEditorAssetLibrary::SetMetadataTag(Asset, METADATA_TAG_SPREADSHEET_ID, SpreadsheetId);
}

FString FGSBAssetBase::GetSpreadsheetId() const
{
	check(Asset);

	FString Value = UEditorAssetLibrary::GetMetadataTag(Asset, METADATA_TAG_SPREADSHEET_ID);

	if (Value.IsEmpty())
	{
		const UGoogleSheetsBridgeSettings* Settings = GetDefault<UGoogleSheetsBridgeSettings>();
		UEditorAssetLibrary::SetMetadataTag(Asset, METADATA_TAG_SPREADSHEET_ID, Settings->DefaultSpreadsheetId);
		return UEditorAssetLibrary::GetMetadataTag(Asset, METADATA_TAG_SPREADSHEET_ID);
	}

	return Value;
}
