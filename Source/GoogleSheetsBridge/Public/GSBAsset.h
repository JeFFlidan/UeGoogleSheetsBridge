// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBUtils.h"
#include "CoreMinimal.h"

class FGSBAsset
{
public:
	FGSBAsset() = default;
	FGSBAsset(UObject* InAsset) : Asset(InAsset) { }
	
	~FGSBAsset() = default;

	UObject* GetHandle() const { return Asset; }
	FName GetFName() const { return Asset->GetFName(); }
	
	void SetSpreadsheetId(const FString& SpreadsheetId);
	FString FindOrAddSpreadsheetId() const;
	FString FindSpreadsheetId() const;
	
	bool ExportToCSVString(FString& OutString);
	bool ImportFromCSVString(const FString& InCSVData);
	
protected:
	UObject* Asset{nullptr};
};
