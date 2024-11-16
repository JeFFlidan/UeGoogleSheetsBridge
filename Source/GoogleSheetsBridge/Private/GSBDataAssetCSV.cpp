// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBDataAssetCSV.h"
#include "GoogleSheetsBridgeLogChannels.h"

FGSBDataAssetExporterCSV::FGSBDataAssetExporterCSV(FString& OutExportedText)
	: ExportedText(OutExportedText)
{
	
}


bool FGSBDataAssetExporterCSV::WriteDataAsset(const UDataAsset* InDataAsset)
{
	DataAsset = InDataAsset;
	
	for (TFieldIterator<FProperty> It(DataAsset->GetClass()); It; ++It)
	{
		FProperty* GeneralProperty = *It;

		if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(GeneralProperty))
		{
			if (!IsCollectionElementPropertyTypeValid(ArrayProperty->Inner))
			{
				UE_LOG(LogGoogleSheetsBridge, Warning, TEXT("Array inner is not UStruct or UObject."))
				continue;
			}
	
			return WriteArrayData(ArrayProperty);
		}

		if (FMapProperty* MapProperty = CastField<FMapProperty>(GeneralProperty))
		{
			if (!IsCollectionElementPropertyTypeValid(MapProperty->ValueProp))
			{
				UE_LOG(LogGoogleSheetsBridge, Warning, TEXT("Map value is not UStruct or UObject."))
				continue;
			}

			return WriteMapData(MapProperty);
		}

		if (FSetProperty* SetProperty = CastField<FSetProperty>(GeneralProperty))
		{
			if (!IsCollectionElementPropertyTypeValid(SetProperty->ElementProp))
			{
				UE_LOG(LogGoogleSheetsBridge, Warning, TEXT("Set element is not UStruct or UObject."))
				continue;
			}

			return WriteSetData(SetProperty);
		}
	}

	return false;
}

bool FGSBDataAssetExporterCSV::WriteArrayData(FArrayProperty* ArrayProperty)
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

bool FGSBDataAssetExporterCSV::WriteMapData(FMapProperty* MapProperty)
{
	return true;
}

bool FGSBDataAssetExporterCSV::WriteSetData(FSetProperty* SetProperty)
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

bool FGSBDataAssetExporterCSV::FindAllSerializableProperties(FProperty* ElementProperty)
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

bool FGSBDataAssetExporterCSV::IsCollectionElementPropertyTypeValid(FProperty* Property) const
{
	if (Property->IsA(FStructProperty::StaticClass())
		|| Property->IsA(FObjectProperty::StaticClass()))
	{
		return true;
	}

	return false;
}

bool FGSBDataAssetExporterCSV::IsPropertySerializable(FProperty* Property) const
{
	if (Property->IsA(FNumericProperty::StaticClass())
		|| Property->IsA(FStrProperty::StaticClass()))
	{
		return true;
	}

	return false;
}
