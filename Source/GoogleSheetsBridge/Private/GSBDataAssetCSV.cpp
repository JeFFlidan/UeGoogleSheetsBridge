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
	SerializableProperties.Reset();

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
	if (Property->IsA(FStructProperty::StaticClass())
		|| Property->IsA(FObjectProperty::StaticClass()))
	{
		return true;
	}

	return false;
}

bool GSB::FGSBDataAssetCSV::IsPropertySerializable(FProperty* Property) const
{
	if (Property->IsA(FNumericProperty::StaticClass())
		|| Property->IsA(FStrProperty::StaticClass()))
	{
		return true;
	}

	return false;
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
		const void* ElementPtr = ArrayHelper.GetRawPtr(Index);

		for (FProperty* Property : SerializableProperties)
		{
			const void* PropertyValue = Property->ContainerPtrToValuePtr<void>(ElementPtr);
			WritePropertyValue(Property, PropertyValue);
		}

		ExportedText.RemoveFromEnd(TEXT(","));
		ExportedText += TEXT("\n");
	}

	return true;
}

bool FGSBDataAssetExporterCSV::HandleMapData(FMapProperty* MapProperty)
{
	return true;
}

bool FGSBDataAssetExporterCSV::HandleSetData(FSetProperty* SetProperty)
{
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

	if (Rows.Num() - 1 > ArrayHelper.Num())
	{
		ArrayHelper.AddValues(Rows.Num() - ArrayHelper.Num() - 1);
	}

	if (Rows.Num() - 1 < ArrayHelper.Num())
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("CSV data has less rows than a Data Asset %s"), *DataAsset->GetName())
		return false;
	}

	for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
	{
		void* ElementPtr = ArrayHelper.GetRawPtr(Index);
		const TArray<const TCHAR*>& Cells = Rows[Index + 1];

		if (SerializableProperties.Num() != Cells.Num())
		{
			UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Row '%d' has more or less cells than properties, is there a malformed string?"), Index)
			continue;
		}

		for (int32 CellIdx = 0; CellIdx < Cells.Num(); ++CellIdx)
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
	}
	
	return true;
}

bool FGSBDataAssetImporterCSV::HandleMapData(FMapProperty* MapProperty)
{
	return FGSBDataAssetCSV::HandleMapData(MapProperty);
}

bool FGSBDataAssetImporterCSV::HandleSetData(FSetProperty* SetProperty)
{
	return FGSBDataAssetCSV::HandleSetData(SetProperty);
}
