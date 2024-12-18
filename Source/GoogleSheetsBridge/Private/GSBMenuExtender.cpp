﻿// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBMenuExtender.h"
#include "GoogleSheetsApi.h"
#include "GSBUtils.h"
#include "GSBConstants.h"
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

	if (bAddMenuEntry_ImportFromCSV)
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString("Import from CSV"),
			FText::FromString("Import from CSV"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FGSBMenuExtenderBase::OpenExplorerToImportCSV))
		);
	}
	
	MenuBuilder.AddMenuSeparator();

	MenuBuilder.AddMenuEntry(
		FText::FromString(GSB::Constants::SyncWithSpreadsheetButtonName),
		FText::FromString(GSB::Constants::SyncWithSpreadsheetTooltip),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]
		{
			FGSBUtils::GenericRequest_GET(SelectedAsset);
		}), FCanExecuteAction::CreateLambda([this]()->bool
		{
			return FGSBUtils::IsSyncButtonExecutable(SelectedAsset);
		}))
	);
	
	MenuBuilder.AddMenuEntry(
		FText::FromString(GSB::Constants::ExportToSpreadsheetButtonName),
		FText::FromString(GSB::Constants::ExportToSpreadsheetTooltip),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]
		{
			FGSBUtils::GenericRequest_POST(SelectedAsset);
		}))
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(GSB::Constants::ChangeSpreadsheetIDButtonName),
		FText::FromString(GSB::Constants::ExportToSpreadsheetTooltip),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]
		{
			FGSBUtils::CreateExportWindow(SelectedAsset);
		}))
	);
}

void FGSBMenuExtenderBase::OpenExplorerToSaveCSV()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	const FString FileTypes = TEXT("Data Table CSV (*.csv)|*.csv");
	
	TArray<FString> OutFilenames;
	DesktopPlatform->SaveFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		TEXT("Export as CSV"),
		TEXT(""),
		TEXT("Report.csv"),
		FileTypes,
		EFileDialogFlags::None,
		OutFilenames
	);

	if (!OutFilenames.IsEmpty())
	{
		FString CsvString;
		if (SelectedAsset.ExportToCSVString(CsvString))
		{
			FFileHelper::SaveStringToFile(CsvString, *OutFilenames[0]);
		}
		else
		{
			UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Failed to export %s as csv"), *SelectedAsset.GetFName().ToString());
		}
	}
}

void FGSBMenuExtenderBase::OpenExplorerToImportCSV()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	const FString FileTypes = TEXT("Data Table CSV (*.csv)|*.csv");

	TArray<FString> OutFilenames;
	DesktopPlatform->OpenFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		TEXT("Import from CSV"),
		TEXT(""),
		TEXT(""),
		FileTypes,
		EFileDialogFlags::None,
		OutFilenames);

	if (!OutFilenames.IsEmpty())
	{
		FString CSVData;
		FFileHelper::LoadFileToString(CSVData, *OutFilenames[0]);
		
		if (!SelectedAsset.ImportFromCSVString(CSVData))
		{
			UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Failed to import %s from csv %s"),
				*SelectedAsset.GetFName().ToString(), *OutFilenames[0]);
		}
	}
}
