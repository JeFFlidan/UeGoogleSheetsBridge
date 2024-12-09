// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UDataAsset;

namespace GSB
{
	class GOOGLESHEETSBRIDGE_API FGSBDataAssetCSV
	{
	public:
		virtual ~FGSBDataAssetCSV() = default;
		
	protected:
		TArray<FProperty*> SerializableProperties;

		bool HandleDataAsset(UClass* DataAssetClass);
		
		virtual bool HandleArrayData(FArrayProperty* ArrayProperty) { return false; }
		virtual bool HandleMapData(FMapProperty* MapProperty) { return false; }
		virtual bool HandleSetData(FSetProperty* SetProperty) { return false; }
		
		bool FindAllSerializableProperties(FProperty* ElementProperty);
	
		bool IsCollectionElementPropertyTypeValid(FProperty* Property) const;
		bool IsPropertySerializable(FProperty* Property) const;
		bool IsMapKeyValid(const FMapProperty* MapProperty, const UDataAsset* DataAsset) const;
	};
}

/**
 * 
 */
class GOOGLESHEETSBRIDGE_API FGSBDataAssetExporterCSV : public GSB::FGSBDataAssetCSV
{
public:
	FGSBDataAssetExporterCSV(FString& OutExportedText);
	bool WriteDataAsset(const UDataAsset* InDataAsset);

private:
	const UDataAsset* DataAsset;
	FString& ExportedText;

	virtual bool HandleArrayData(FArrayProperty* ArrayProperty) override;
	virtual bool HandleMapData(FMapProperty* MapProperty) override;
	virtual bool HandleSetData(FSetProperty* SetProperty) override;
	
	bool WriteHeader(FProperty* ElementProperty);
	void WriteRaw(const void* ElementPtr);
	void WritePropertyValue(FProperty* Property, const void* PropertyValue);
};

/**
 * 
 */
class GOOGLESHEETSBRIDGE_API FGSBDataAssetImporterCSV : public GSB::FGSBDataAssetCSV
{
public:
	FGSBDataAssetImporterCSV(UDataAsset* InDataAsset, const FString& InCSVData);

	bool ReadDataAsset();

private:
	UDataAsset* DataAsset{nullptr};
	const FString& CSVData;
	
	virtual bool HandleArrayData(FArrayProperty* ArrayProperty) override;
	virtual bool HandleMapData(FMapProperty* MapProperty) override;
	virtual bool HandleSetData(FSetProperty* SetProperty) override;

	void FillCell(void* ElementPtr, int32 CellIdx, const TArray<const TCHAR*>& Cells);
	bool IsCellCountValid(int32 CellCount, int32 Index) const;
	bool IsRowCountValid(int32 RowFromCSVCount, int32 ElementCount) const;
};
