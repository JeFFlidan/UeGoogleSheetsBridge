// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GoogleSheetsBridge.h"
#include "GoogleSheetsApi.h"
#include "GSBMenuExtender.h"
#include "GSBToolBarExtender.h"
#include "GSBUtils.h"
#include "GoogleSheetsBridgeLogChannels.h"

#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "FGoogleSheetsBridgeModule"

void FGoogleSheetsBridgeModule::StartupModule()
{
	MenuExtenders.Push(MakeUnique<TGSBMenuExtender<UDataAsset>>());
	MenuExtenders.Last()->AddMenuEntry_ExportToCSV();
	MenuExtenders.Last()->AddMenuEntry_ImportFromCSV();
	
	MenuExtenders.Push(MakeUnique<TGSBMenuExtender<UCurveTable>>());
	MenuExtenders.Push(MakeUnique<TGSBMenuExtender<UDataTable>>());
	
	for (TUniquePtr<FGSBMenuExtenderBase>& MenuExtender : MenuExtenders)
	{
		MenuExtender->Initialize();
	}

	FGSBToolBarExtender::Initialize();

	FGoogleSheetsApi::SetEnableNotifications(false);
	
	SyncAssetsWithGoogleSheets(UDataAsset::StaticClass());
	SyncAssetsWithGoogleSheets(UDataTable::StaticClass());
	SyncAssetsWithGoogleSheets(UCurveTable::StaticClass());

	FGoogleSheetsApi::SetEnableNotifications(true);
}

void FGoogleSheetsBridgeModule::ShutdownModule()
{
	for (TUniquePtr<FGSBMenuExtenderBase>& MenuExtender : MenuExtenders)
	{
		MenuExtender->Uninitialize();
	}
}

void FGoogleSheetsBridgeModule::SyncAssetsWithGoogleSheets(UClass* Class)
{
	AsyncTask(ENamedThreads::GameThread, [Class]
	{
		UE_LOG(LogGoogleSheetsBridge, Display, TEXT("Started synchronizing %s with Google Sheets"), *Class->GetFName().ToString())
		
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		TArray<FAssetData> AssetsData;
		AssetRegistry.GetAssetsByClass(Class->GetClassPathName(), AssetsData, true);

		for (FAssetData& AssetData : AssetsData)
		{
			FGSBAsset Asset(AssetData.GetAsset());
			
			if (Asset.FindSpreadsheetId().IsEmpty())
			{
			    continue;
			}

			FGSBUtils::GenericRequest_GET(Asset);
		}
	});
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGoogleSheetsBridgeModule, GoogleSheetsBridge)