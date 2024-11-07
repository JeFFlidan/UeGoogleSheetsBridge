// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UDataAsset;

/**
 * 
 */
class GOOGLESHEETSBRIDGE_API FDataAssetExporterCSV
{
public:
	FDataAssetExporterCSV(FString& OutExportText);
	~FDataAssetExporterCSV();

	bool WriteDataAsset(const UDataAsset* InDataAsset);

private:
	bool WriteArrayData(FArrayProperty* ArrayProperty);
	bool WriteMapData(FMapProperty* MapProperty);
	bool WriteSetData(FSetProperty* SetProperty);
	bool WriteHeader(FProperty* ElementProperty);
	void WritePropertyValue(FProperty* Property, const void* PropertyValue);

	bool FindAllSerializableProperties(FProperty* ElementProperty);
	
	bool IsCollectionElementPropertyTypeValid(FProperty* Property) const;
	bool IsPropertySerializable(FProperty* Property) const;

	FString& ExportedText;
	const UDataAsset* DataAsset;
	TArray<FProperty*> SerializableProperties;
};

/**
 * 
 */
class GOOGLESHEETSBRIDGE_API FDataAssetImporterCSV
{
public:
	
};
