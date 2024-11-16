// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBDataAssetCSV.h"
#include "CoreMinimal.h"
#include "Kismet/DataTableFunctionLibrary.h"

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
}
