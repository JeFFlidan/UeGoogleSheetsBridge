// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBDataAssetCSV.h"
#include "GoogleSheetsBridgeLogChannels.h"
#include "Serialization/Csv/CsvParser.h"

bool GSB::FGSBDataAssetCSV::HandleDataAsset(UClass* DataAssetClass)
{
	for (TFieldIterator<FProperty> It(DataAssetClass); It; ++It)
	{
		FProperty* GeneralProperty = *It;

		if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(GeneralProperty))
		{
			if (!IsCollectionElementPropertyTypeValid(ArrayProperty->Inner))
			{
				UE_LOG(LogGoogleSheetsBridge, Warning, TEXT("Array inner is not UStruct or UObject."))
				continue;
			}
	
			return HandleArrayData(ArrayProperty);
		}

		if (FMapProperty* MapProperty = CastField<FMapProperty>(GeneralProperty))
		{
			if (!IsCollectionElementPropertyTypeValid(MapProperty->ValueProp))
			{
				UE_LOG(LogGoogleSheetsBridge, Warning, TEXT("Map value is not UStruct or UObject."))
				continue;
			}

			return HandleMapData(MapProperty);
		}

		if (FSetProperty* SetProperty = CastField<FSetProperty>(GeneralProperty))
		{
			if (!IsCollectionElementPropertyTypeValid(SetProperty->ElementProp))
			{
				UE_LOG(LogGoogleSheetsBridge, Warning, TEXT("Set element is not UStruct or UObject."))
				continue;
			}

			return HandleSetData(SetProperty);
		}
	}

	return false;
}

bool GSB::FGSBDataAssetCSV::FindAllSerializableProperties(FProperty* ElementProperty)
{
	if (FStructProperty* StructProperty = CastField<FStructProperty>(ElementProperty))
	{
		for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
		{
			FProperty* Property = *It;

			if (IsPropertySerializable(Property))
			{
				SerializableProperties.Add(Property);
			}
		}
		
		return !SerializableProperties.IsEmpty();
	}

	return false;
}

bool GSB::FGSBDataAssetCSV::IsCollectionElementPropertyTypeValid(FProperty* Property) const
{
	if (Property->IsA(FStructProperty::StaticClass()))
	{
		return true;
	}

	return false;
}

bool GSB::FGSBDataAssetCSV::IsPropertySerializable(FProperty* Property) const
{
	if (Property->IsA(FNumericProperty::StaticClass())
		|| Property->IsA(FStrProperty::StaticClass())
		|| Property->IsA(FBoolProperty::StaticClass()))
	{
		return true;
	}

	return false;
}

bool GSB::FGSBDataAssetCSV::IsMapKeyValid(const FMapProperty* MapProperty, const UDataAsset* DataAsset) const
{
	if (!MapProperty->KeyProp->IsA(FStrProperty::StaticClass()))
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Map key in %s is not string."), *DataAsset->GetName())
		return false;
	}

	return true;
}

FGSBDataAssetExporterCSV::FGSBDataAssetExporterCSV(FString& OutExportedText)
	: ExportedText(OutExportedText)
{
	
}

bool FGSBDataAssetExporterCSV::WriteDataAsset(const UDataAsset* InDataAsset)
{
	DataAsset = InDataAsset;
	return HandleDataAsset(DataAsset->GetClass());
}

bool FGSBDataAssetExporterCSV::HandleArrayData(FArrayProperty* ArrayProperty)
{
	if (!WriteHeader(ArrayProperty->Inner))
	{
		return false;
	}
	
	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(DataAsset));
	
	for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
	{
		WriteRaw(ArrayHelper.GetRawPtr(Index));
	}

	return true;
}

bool FGSBDataAssetExporterCSV::HandleMapData(FMapProperty* MapProperty)
{
	if (!IsMapKeyValid(MapProperty, DataAsset))
	{
		return false;
	}
	
	ExportedText += TEXT("---,");
	if (!WriteHeader(MapProperty->ValueProp))
	{
		ExportedText.Empty();
		return false;
	}

	FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(DataAsset));

	for (int32 Index = 0; Index < MapHelper.Num(); ++Index)
	{
		if (!MapHelper.IsValidIndex(Index))
		{
			continue;
		}

		WritePropertyValue(MapHelper.KeyProp, MapHelper.GetKeyPtr(Index));
		WriteRaw(MapHelper.GetValuePtr(Index));
	}
	
	return true;
}

bool FGSBDataAssetExporterCSV::HandleSetData(FSetProperty* SetProperty)
{
	if (!WriteHeader(SetProperty->ElementProp))
	{
		return false;
	}
	
	FScriptSetHelper SetHelper(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(DataAsset));

	for (int32 Index = 0; Index < SetHelper.Num(); ++Index)
	{
		if (!SetHelper.IsValidIndex(Index))
		{
			continue;
		}

		WriteRaw(SetHelper.GetElementPtr(Index));
	}
	
	return true;
}

bool FGSBDataAssetExporterCSV::WriteHeader(FProperty* ElementProperty)
{
	if (!FindAllSerializableProperties(ElementProperty))
	{
		return false;
	}

	for (FProperty* Property : SerializableProperties)
	{
		ExportedText += Property->GetAuthoredName();
		ExportedText += TEXT(",");
	}

	ExportedText.RemoveFromEnd(",");
	ExportedText += TEXT("\n");

	return true;
}

void FGSBDataAssetExporterCSV::WriteRaw(const void* ElementPtr)
{
	for (FProperty* Property : SerializableProperties)
	{
		const void* PropertyValue = Property->ContainerPtrToValuePtr<void>(ElementPtr);
		WritePropertyValue(Property, PropertyValue);
	}

	ExportedText.RemoveFromEnd(TEXT(","));
    ExportedText += TEXT("\n");
}

void FGSBDataAssetExporterCSV::WritePropertyValue(FProperty* Property, const void* PropertyValue)
{
	FString StrValue;
	Property->ExportText_Direct(StrValue, PropertyValue, PropertyValue, nullptr, PPF_ExternalEditor);

	ExportedText += TEXT("\"");
	ExportedText += StrValue.Replace(TEXT("\""), TEXT("\"\""));
	ExportedText += TEXT("\"");
	ExportedText += TEXT(",");
}

FGSBDataAssetImporterCSV::FGSBDataAssetImporterCSV(UDataAsset* InDataAsset, const FString& InCSVData)
	: DataAsset(InDataAsset), CSVData(InCSVData)
{
}

bool FGSBDataAssetImporterCSV::ReadDataAsset()
{
	return HandleDataAsset(DataAsset->GetClass());
}

bool FGSBDataAssetImporterCSV::HandleArrayData(FArrayProperty* ArrayProperty)
{
	if (!FindAllSerializableProperties(ArrayProperty->Inner))
	{
		return false;
	}
	
	FCsvParser Parser(CSVData);
	const auto& Rows = Parser.GetRows();

	if (Rows.IsEmpty())
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("CSV data is empty"))
		return false;
	}
	
	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(DataAsset));

	if (!IsRowCountValid(Rows.Num(), ArrayHelper.Num()))
	{
		return false;
	}
	
	if (Rows.Num() - 1 > ArrayHelper.Num())
	{
		ArrayHelper.AddValues(Rows.Num() - ArrayHelper.Num() - 1);
	}

	for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
	{
		void* ElementPtr = ArrayHelper.GetRawPtr(Index);
		const TArray<const TCHAR*>& Cells = Rows[Index + 1];

		if (!IsCellCountValid(Cells.Num(), Index))
		{
			continue;
		}

		for (int32 CellIdx = 0; CellIdx < Cells.Num(); ++CellIdx)
		{
			FillCell(ElementPtr, CellIdx, Cells);
		}
	}
	
	return true;
}

bool FGSBDataAssetImporterCSV::HandleMapData(FMapProperty* MapProperty)
{
	if (!IsMapKeyValid(MapProperty, DataAsset))
	{
		return false;
	}

	SerializableProperties.Add(MapProperty->KeyProp);
	FindAllSerializableProperties(MapProperty->ValueProp);
	if (SerializableProperties.Num() - 1 <= 0)
	{
		return false;
	}

	FCsvParser Parser(CSVData);
	const auto& Rows = Parser.GetRows();

	if (Rows.IsEmpty())
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("CSV data is empty"))
		return false;
	}
	
	FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(DataAsset));

	if (!IsRowCountValid(Rows.Num(), MapHelper.Num()))
	{
		return false;
	}

	for (int32 Index = 0; Index < Rows.Num() - 1; ++Index)
	{
		const TArray<const TCHAR*>& Cells = Rows[Index + 1];
		void* ElementPtr = nullptr;

		if (!IsCellCountValid(Cells.Num(), Index))
		{
			continue;
		}
		
		if (MapHelper.IsValidIndex(Index))
		{
			FillCell(MapHelper.GetKeyPtr(Index), 0, Cells);
			ElementPtr = MapHelper.GetValuePtr(Index);
		}
		else
		{
			int32 PairIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
			MapHelper.Rehash();
			FillCell(MapHelper.GetKeyPtr(PairIndex), 0, Cells);
			ElementPtr = MapHelper.GetValuePtr(PairIndex);
		}

		for (int32 CellIdx = 1; CellIdx < Cells.Num(); ++CellIdx)
		{
			FillCell(ElementPtr, CellIdx, Cells);
		}
	}
	
	return true;
}

bool FGSBDataAssetImporterCSV::HandleSetData(FSetProperty* SetProperty)
{
	if (!FindAllSerializableProperties(SetProperty->ElementProp))
	{
		return false;
	}
	
	FCsvParser Parser(CSVData);
	const auto& Rows = Parser.GetRows();

	if (Rows.IsEmpty())
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("CSV data is empty"))
		return false;
	}

	FScriptSetHelper SetHelper(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(DataAsset));

	if (!IsRowCountValid(Rows.Num(), SetHelper.Num()))
	{
		return false;
	}

	for (int32 Index = 0; Index < Rows.Num() - 1; ++Index)
	{
		const TArray<const TCHAR*>& Cells = Rows[Index + 1];
		void* ElementPtr = nullptr;

		if (!IsCellCountValid(Cells.Num(), Index))
		{
			continue;
		}

		if (SetHelper.IsValidIndex(Index))
		{
			ElementPtr = SetHelper.GetElementPtr(Index);
		}
		else
		{
			int32 ElementIndex = SetHelper.AddUninitializedValue();
			SetHelper.Rehash();
			ElementPtr = SetHelper.GetElementPtr(ElementIndex);
		}

		for (int32 CellIdx = 0; CellIdx < Cells.Num(); ++CellIdx)
		{
			FillCell(ElementPtr, CellIdx, Cells);
		}
	}
	
	return true;
}

void FGSBDataAssetImporterCSV::FillCell(void* ElementPtr, int32 CellIdx, const TArray<const TCHAR*>& Cells)
{
	FProperty* ColumnProperty = SerializableProperties[CellIdx];
	void* PropertyValue = ColumnProperty->ContainerPtrToValuePtr<void>(ElementPtr);
	const TCHAR* CellValue = Cells[CellIdx];

	FStringOutputDevice OutputDevice;
	ColumnProperty->ImportText_Direct(CellValue, PropertyValue, nullptr, PPF_ExternalEditor, &OutputDevice);

	FString Error(OutputDevice);
	if (!Error.IsEmpty())
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("%s"), *Error);
	}
}

bool FGSBDataAssetImporterCSV::IsCellCountValid(int32 CellCount, int32 Index) const
{
	if (SerializableProperties.Num() != CellCount)
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Row '%d' has more or less cells than properties, is there a malformed string?"), Index)
		return false;
	}

	return true;
}

bool FGSBDataAssetImporterCSV::IsRowCountValid(int32 RowFromCSVCount, int32 ElementCount) const
{
	if (RowFromCSVCount - 1 < ElementCount)
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("CSV data has less rows than a Data Asset %s"), *DataAsset->GetName())
		return false;
	}

	return true;
}
