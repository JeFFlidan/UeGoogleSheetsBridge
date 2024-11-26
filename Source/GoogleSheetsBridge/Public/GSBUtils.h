// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBDataAssetCSV.h"
#include "GoogleSheetsBridgeLogChannels.h"

#include "CoreMinimal.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "DataTableEditorUtils.h"
#include "CurveTableEditorUtils.h"

namespace GSB
{
	template<typename AssetType>
	bool AssetToCsvString(const AssetType* Asset, FString& OutString)
	{
		static_assert(!TIsSame<AssetType, AssetType>::Value, "Type is not supported");
		return false;
	}

	template<>
	inline bool AssetToCsvString(const UCurveTable* Asset, FString& OutString)
	{
		OutString = Asset->GetTableAsCSV();
		return !OutString.IsEmpty();
	}

	template<>
	inline bool AssetToCsvString(const UDataAsset* Asset, FString& OutString)
	{
		return FGSBDataAssetExporterCSV(OutString).WriteDataAsset(Asset);
	}

	template<>
	inline bool AssetToCsvString(const UDataTable* Asset, FString& OutString)
	{
		return UDataTableFunctionLibrary::ExportDataTableToCSVString(Asset, OutString);
	}

	template<typename AssetType>
	bool CSVStringToAsset(AssetType* Asset, const FString& CSVData)
	{
		static_assert(!TIsSame<AssetType, AssetType>::Value, "Type is not supported");
		return false;
	}

	template<>
	inline bool CSVStringToAsset(UCurveTable* Asset, const FString& CSVData)
	{
		TArray<FString> Errors = Asset->CreateTableFromCSVString(CSVData);

		if (!Errors.IsEmpty())
		{
			for (const FString& Error : Errors)
			{
				UE_LOG(LogGoogleSheetsBridge, Error, TEXT("%s"), *Error);
			}

			return false;
		}

		FCurveTableEditorUtils::BroadcastPostChange(Asset, FCurveTableEditorUtils::ECurveTableChangeInfo::RowList);

		return true;
	}

	template<>
	inline bool CSVStringToAsset(UDataAsset* Asset, const FString& CSVData)
	{
		return FGSBDataAssetImporterCSV(Asset, CSVData).ReadDataAsset();
	}

	template<>
	inline bool CSVStringToAsset(UDataTable* Asset, const FString& CSVData)
	{
		bool bResult = UDataTableFunctionLibrary::FillDataTableFromCSVString(Asset, CSVData);
		FDataTableEditorUtils::BroadcastPostChange(Asset, FDataTableEditorUtils::EDataTableChangeInfo::RowList);
		return bResult;
	}
}
