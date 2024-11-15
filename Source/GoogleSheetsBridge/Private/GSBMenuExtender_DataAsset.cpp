// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBMenuExtender_DataAsset.h"
#include "GSBDataAssetCSV.h"
#include "GoogleSheetsApi.h"
#include "GoogleSheetsBridgeLogChannels.h"

#include "Framework/Application/SlateApplication.h"
#include "Engine/DataAsset.h"
#include "DesktopPlatformModule.h"

bool FGSBMenuExtender_DataAsset::SetSelectedAsset(const TArray<FAssetData>& AssetDataList)
{
	if (AssetDataList.Num() != 1 || !AssetDataList[0].GetClass()->IsChildOf(UDataAsset::StaticClass()))
		return false;

	SelectedDataAsset = Cast<UDataAsset>(AssetDataList[0].GetAsset());
	
	return true;
}

void FGSBMenuExtender_DataAsset::AddMenuEntries(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Export to CSV"),
		FText::FromString("Export to CSV"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FGSBMenuExtender_DataAsset::OpenExplorerToSave))
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Export and Connect to Google Sheets"),
		FText::FromString("Export and Connect to Google Sheets"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FGSBMenuExtender_DataAsset::ExportToGoogleSheets))
	);
}

void FGSBMenuExtender_DataAsset::OpenExplorerToSave()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	const FString FileTypes = TEXT("Data Table CSV (*.csv)|*.csv");

	TArray<FString> OutFilenames;
	DesktopPlatform->SaveFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		TEXT("Export to CSV"),
		TEXT(""),
		TEXT("Report.csv"),
		FileTypes,
		EFileDialogFlags::None,
		OutFilenames
	);

	if (!OutFilenames.IsEmpty())
	{
		if (ExporterCSV.WriteDataAsset(SelectedDataAsset))
		{
			FFileHelper::SaveStringToFile(ExporterCSV.GetCSV(), *OutFilenames[0]);
		}
		else
		{
			UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Failed to export data asset %s as csv"), *SelectedDataAsset->GetFName().ToString());
		}
	}
}

void FGSBMenuExtender_DataAsset::ExportToGoogleSheets()
{
	if (ExporterCSV.WriteDataAsset(SelectedDataAsset))
	{
		FGoogleSheetsApiParams_POST Params(
			GetSpreadsheetId(SelectedDataAsset),
			SelectedDataAsset->GetName(),
			ExporterCSV.GetCSV());

		UGoogleSheetsApi* GoogleSheetsApi = NewObject<UGoogleSheetsApi>();
		GoogleSheetsApi->OnResponseReceived_POST.AddLambda([this](FString Content)
		{
			// Because of bug with bad request response, does not work correctly for now
			UE_LOG(LogGoogleSheetsBridge, Verbose, TEXT("%s"), *Content);
		});
		GoogleSheetsApi->SendPostRequest(Params);
	}
}
