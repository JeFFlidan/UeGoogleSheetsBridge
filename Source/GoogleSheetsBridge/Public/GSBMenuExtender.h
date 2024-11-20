// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBAsset.h"
#include "CoreMinimal.h"

class FGSBMenuExtenderBase
{
public:
	virtual ~FGSBMenuExtenderBase() = default;

	void Initialize();
	void Uninitialize();

	// Must be called before Initialize
	void AddMenuEntry_ExportToCSV() { bAddMenuEntry_ExportToCSV = true; }
	
	// Must be called before Initialize
	void AddMenuEntry_ImportFromCSV() { bAddMenuEntry_ImportFromCSV = true; }

protected:
	FDelegateHandle MenuExtenderDelegateHandle;
	TUniquePtr<FGSBAssetBase> SelectedAsset;

	bool bAddMenuEntry_ExportToCSV{false};
	bool bAddMenuEntry_ImportFromCSV{false};

	TSharedRef<FExtender> ExtendContextMenu(const TArray<FAssetData>& AssetDataList);
	void ExportToGoogleSheets();
	void OpenExplorerToSaveCSV();
	void OpenExplorerToImportCSV();

	virtual bool SetSelectedAsset(const TArray<FAssetData>& AssetDataList) { return false; }
	virtual void AddMenuEntries(FMenuBuilder& MenuBuilder);
};

template<typename AssetType>
class TGSBMenuExtender : public FGSBMenuExtenderBase
{
protected:
	virtual bool SetSelectedAsset(const TArray<FAssetData>& AssetDataList) override;
};

template<typename AssetType>
bool TGSBMenuExtender<AssetType>::SetSelectedAsset(const TArray<FAssetData>& AssetDataList)
{
	if (AssetDataList.Num() != 1 || !AssetDataList[0].GetClass()->IsChildOf(AssetType::StaticClass()))
		return false;
	
	SelectedAsset.Reset(new TGSBAsset<AssetType>(AssetDataList[0].GetAsset()));
	
	return true;
}
