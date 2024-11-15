// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBMenuExtender.h"
#include "GSBDataAssetCSV.h"
#include "CoreMinimal.h"

class UDataAsset;

class GOOGLESHEETSBRIDGE_API FGSBMenuExtender_DataAsset final : public FGSBMenuExtender
{
	FGSBDataAssetExporterCSV ExporterCSV;
	UDataAsset* SelectedDataAsset{nullptr};
	
	virtual bool SetSelectedAsset(const TArray<FAssetData>& AssetDataList) override;
	virtual void AddMenuEntries(FMenuBuilder& MenuBuilder) override;
	
	void OpenExplorerToSave();
	void ExportToGoogleSheets();
};
