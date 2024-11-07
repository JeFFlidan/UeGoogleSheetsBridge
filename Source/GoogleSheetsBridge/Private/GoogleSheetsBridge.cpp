// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GoogleSheetsBridge.h"
#include "DataAssetCSV.h"
#include "GoogleSheetsBridgeLogChannels.h"

#include "Framework/Application/SlateApplication.h"
#include "Engine/DataAsset.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"
#include "DesktopPlatformModule.h"

#define LOCTEXT_NAMESPACE "FGoogleSheetsBridgeModule"

void FGoogleSheetsBridgeModule::StartupModule()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& AssetMenuExtendersDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	AssetMenuExtendersDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FGoogleSheetsBridgeModule::DataAssetContextMenuExtender));
	AssetMenuExtenderDelegateHandle = AssetMenuExtendersDelegates.Last().GetHandle();
}

void FGoogleSheetsBridgeModule::ShutdownModule()
{
	if (!IsRunningCommandlet() && !IsRunningGame())
	{
		FContentBrowserModule* ContentBrowserModule = static_cast<FContentBrowserModule*>(FModuleManager::Get().GetModule("ContentBrowser"));
		if (ContentBrowserModule)
		{
			TArray<FContentBrowserMenuExtender_SelectedAssets>& AssetMenuExtendersDelegates = ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
			AssetMenuExtendersDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate)
			{
				return Delegate.GetHandle() == AssetMenuExtenderDelegateHandle;
			});
		}
	}
}

void FGoogleSheetsBridgeModule::AddMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Export to CSV"),
		FText::FromString("Export to CSV"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FGoogleSheetsBridgeModule::OpenExplorerToSave))
	);
}

void FGoogleSheetsBridgeModule::OpenExplorerToSave()
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
		FString SaveString;
		if (FDataAssetExporterCSV(SaveString).WriteDataAsset(SelectedDataAsset))
		{
			FFileHelper::SaveStringToFile(SaveString, *OutFilenames[0]);
		}
		else
		{
			UE_LOG(LogGoogleSheetsBridge, Error, TEXT("Failed to export data asset %s as csv"), *SelectedDataAsset->GetFName().ToString());
		}
	}
}

TSharedRef<FExtender> FGoogleSheetsBridgeModule::DataAssetContextMenuExtender(const TArray<FAssetData>& AssetDataList)
{
	if (AssetDataList.Num() != 1 || !AssetDataList[0].GetClass()->IsChildOf(UDataAsset::StaticClass()))
		return MakeShareable(new FExtender());

	SelectedDataAsset = Cast<UDataAsset>(AssetDataList[0].GetAsset());

	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());

	MenuExtender->AddMenuExtension(
		"GetAssetActions",
		EExtensionHook::After,
		TSharedPtr<FUICommandList>(),
		FMenuExtensionDelegate::CreateRaw(this, &FGoogleSheetsBridgeModule::AddMenuEntry));

	return MenuExtender.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGoogleSheetsBridgeModule, GoogleSheetsBridge)