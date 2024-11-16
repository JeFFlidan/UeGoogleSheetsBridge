// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBUtils.h"
#include "CoreMinimal.h"

class FGSBMenuExtenderBase
{
public:
	virtual ~FGSBMenuExtenderBase() = default;

	void Initialize();
	void Uninitialize();

	// Must be called before Initialize
	void AddMenuEntry_ExportToCSV() { bAddMenuEntry_ExportToCSV = true; }

protected:
	FDelegateHandle MenuExtenderDelegateHandle;
	UObject* SelectedAsset{nullptr};

	bool bAddMenuEntry_ExportToCSV{false};

	TSharedRef<FExtender> ExtendContextMenu(const TArray<FAssetData>& AssetDataList);
	const FString& GetSpreadsheetId() const;
	void ExportToGoogleSheets();
	void OpenExplorerToSaveCSV();

	virtual bool SetSelectedAsset(const TArray<FAssetData>& AssetDataList) { return false; }
	virtual void AddMenuEntries(FMenuBuilder& MenuBuilder);
	virtual bool ConvertAssetToCsvString(FString& OutString) { return false; }
};

template<typename AssetType>
class TGSBMenuExtender : public FGSBMenuExtenderBase
{
protected:
	virtual bool SetSelectedAsset(const TArray<FAssetData>& AssetDataList) override;
	virtual bool ConvertAssetToCsvString(FString& OutString) override;
};

template<typename AssetType>
bool TGSBMenuExtender<AssetType>::SetSelectedAsset(const TArray<FAssetData>& AssetDataList)
{
	if (AssetDataList.Num() != 1 || !AssetDataList[0].GetClass()->IsChildOf(AssetType::StaticClass()))
		return false;

	SelectedAsset = AssetDataList[0].GetAsset();
	
	return true;
}

template <typename AssetType>
bool TGSBMenuExtender<AssetType>::ConvertAssetToCsvString(FString& OutString)
{
	return GSB::AssetToCsvString(CastChecked<AssetType>(SelectedAsset), OutString);
}
