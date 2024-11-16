// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBMenuExtender.h"
#include "GoogleSheetsApi.h"
#include "GoogleSheetsBridgeLogChannels.h"

#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/DataAsset.h"
#include "DesktopPlatformModule.h"

void FGSBMenuExtenderBase::Initialize()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& AssetMenuExtendersDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	AssetMenuExtendersDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FGSBMenuExtenderBase::ExtendContextMenu));
	MenuExtenderDelegateHandle = AssetMenuExtendersDelegates.Last().GetHandle();
}

void FGSBMenuExtenderBase::Uninitialize()
{
	if (!IsRunningCommandlet() && !IsRunningGame())
	{
		FContentBrowserModule* ContentBrowserModule = static_cast<FContentBrowserModule*>(FModuleManager::Get().GetModule("ContentBrowser"));
		if (ContentBrowserModule)
		{
			TArray<FContentBrowserMenuExtender_SelectedAssets>& AssetMenuExtendersDelegates = ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
			AssetMenuExtendersDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate)
			{
				return Delegate.GetHandle() == MenuExtenderDelegateHandle;
			});
		}
	}
}

TSharedRef<FExtender> FGSBMenuExtenderBase::ExtendContextMenu(const TArray<FAssetData>& AssetDataList)
{
	if (!SetSelectedAsset(AssetDataList))
	{
		return MakeShareable(new FExtender());
	}

	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());

	MenuExtender->AddMenuExtension(
		"GetAssetActions",
		EExtensionHook::After,
		TSharedPtr<FUICommandList>(),
		FMenuExtensionDelegate::CreateRaw(this, &FGSBMenuExtenderBase::AddMenuEntries));

	return MenuExtender.ToSharedRef();
}

void FGSBMenuExtenderBase::AddMenuEntries(FMenuBuilder& MenuBuilder)
{
	if (bAddMenuEntry_ExportToCSV)
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString("Export as CSV"),
			FText::FromString("Export as CSV"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FGSBMenuExtenderBase::OpenExplorerToSaveCSV))
		);
	}
	
	MenuBuilder.AddMenuEntry(
		FText::FromString("Export to Default Google Sheets"),
		FText::FromString("Export to Default Google Sheets"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FGSBMenuExtenderBase::ExportToGoogleSheets))
	);
}

void FGSBMenuExtenderBase::ExportToGoogleSheets()
{
	FGoogleSheetsApiParams_POST Params(SelectedAsset->GetSpreadsheetId(), SelectedAsset->GetFName());
	if (SelectedAsset->ExportToCSVString(Params.Content))
	{
		UGoogleSheetsApi* GoogleSheetsApi = NewObject<UGoogleSheetsApi>();
		GoogleSheetsApi->OnResponseReceived_POST.AddLambda([this](FString Content)
		{
			// Because of bug with bad request response, does not work correctly for now
			UE_LOG(LogGoogleSheetsBridge, Verbose, TEXT("%s"), *Content);
		});
		GoogleSheetsApi->SendPostRequest(Params);
	}
	else
	{
		UE_LOG(LogGoogleSheetsBridge, Error, TEXT("POST params are invalid."));
	}
}

void FGSBMenuExtenderBase::OpenExplorerToSaveCSV()
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
		FString CsvString;
		if (SelectedAsset->ExportToCSVString(CsvString))
		{
			FFileHelper::SaveStringToFile(CsvString, *OutFilenames[0]);
		}
		else
		{
			UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Failed to export %s as csv"), *SelectedAsset->GetFName().ToString());
		}
	}
}
