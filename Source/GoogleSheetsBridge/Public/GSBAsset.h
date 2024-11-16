// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBUtils.h"
#include "CoreMinimal.h"

class FGSBAssetBase
{
public:
	FGSBAssetBase() = default;
	FGSBAssetBase(UObject* InAsset) : Asset(InAsset) { }
	
	virtual ~FGSBAssetBase() = default;

	UObject* GetHandle() const { return Asset; }
	FName GetFName() const { return Asset->GetFName(); }
	
	void SetSpreadsheetId(const FString& SpreadsheetId);
	FString GetSpreadsheetId() const;
	
	virtual bool ExportToCSVString(FString& OutString) { return false; }
	
protected:
	UObject* Asset{nullptr};
};

template<typename AssetType>
class TGSBAsset : public FGSBAssetBase
{
public:
	TGSBAsset(UObject* InAsset) : FGSBAssetBase(InAsset) {}

	virtual bool ExportToCSVString(FString& OutString) override;
};

template <typename AssetType>
bool TGSBAsset<AssetType>::ExportToCSVString(FString& OutString)
{
	return GSB::AssetToCsvString(CastChecked<AssetType>(Asset), OutString);
}

