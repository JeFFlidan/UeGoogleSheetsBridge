// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FGSBMenuExtender
{
public:
	virtual ~FGSBMenuExtender() = default;

	void Initialize();
	void Uninitialize();
	
protected:
	FDelegateHandle MenuExtenderDelegateHandle;
	
	TSharedRef<FExtender> ExtendContextMenu(const TArray<FAssetData>& AssetDataList);
	const FString& GetSpreadsheetId(UObject* Object) const;

	virtual bool SetSelectedAsset(const TArray<FAssetData>& AssetDataList) = 0;
	virtual void AddMenuEntries(FMenuBuilder& MenuBuilder) = 0;
};
