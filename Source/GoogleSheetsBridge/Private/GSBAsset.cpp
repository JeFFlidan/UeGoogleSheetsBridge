// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBAsset.h"
#include "GSBUtils.h"
#include "GoogleSheetsBridgeSettings.h"

#include "EditorAssetLibrary.h"

const FName METADATA_TAG_SPREADSHEET_ID = "GSB.SpreadsheetId";

void FGSBAsset::SetSpreadsheetId(const FString& SpreadsheetId)
{
	check(Asset);
	UEditorAssetLibrary::SetMetadataTag(Asset, METADATA_TAG_SPREADSHEET_ID, SpreadsheetId);
}

FString FGSBAsset::FindOrAddSpreadsheetId() const
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

FString FGSBAsset::FindSpreadsheetId() const
{
	check(Asset);

	return UEditorAssetLibrary::GetMetadataTag(Asset, METADATA_TAG_SPREADSHEET_ID);
}

bool FGSBAsset::ExportToCSVString(FString& OutString)
{
	if (Asset->GetClass()->IsChildOf<UDataTable>())
	{
		return GSB::AssetToCsvString(CastChecked<UDataTable>(Asset), OutString);
	}

	if (Asset->GetClass()->IsChildOf<UCurveTable>())
	{
		return GSB::AssetToCsvString(CastChecked<UCurveTable>(Asset), OutString);
	}

	if (Asset->GetClass()->IsChildOf<UDataAsset>())
	{
		return GSB::AssetToCsvString(CastChecked<UDataAsset>(Asset), OutString);
	}

	return false;
}

bool FGSBAsset::ImportFromCSVString(const FString& InCSVData)
{
	if (UDataTable* DataTable = Cast<UDataTable>(Asset))
	{
		return GSB::CSVStringToAsset(DataTable, InCSVData);
	}

	if (Asset->GetClass()->IsChildOf<UCurveTable>())
	{
		return GSB::CSVStringToAsset(CastChecked<UCurveTable>(Asset), InCSVData);
	}

	if (Asset->GetClass()->IsChildOf<UDataAsset>())
	{
		return GSB::CSVStringToAsset(CastChecked<UDataAsset>(Asset), InCSVData);
	}

	return false;
}
