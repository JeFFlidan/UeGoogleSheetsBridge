// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBUtils.h"
#include "CoreMinimal.h"

class FGSBAsset
{
public:
	FGSBAsset() = default;
	FGSBAsset(UObject* InAsset) : Asset(InAsset) { }
	
	virtual ~FGSBAsset() = default;

	UObject* GetHandle() const { return Asset; }
	FName GetFName() const { return Asset->GetFName(); }
	
	void SetSpreadsheetId(const FString& SpreadsheetId);
	FString GetSpreadsheetId() const;
	
	bool ExportToCSVString(FString& OutString);
	bool ImportFromCSVString(const FString& InCSVData);
	
protected:
	UObject* Asset{nullptr};
};
