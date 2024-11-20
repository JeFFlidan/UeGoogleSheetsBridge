// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GoogleSheetsBridge.h"
#include "GSBMenuExtender.h"

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
}

void FGoogleSheetsBridgeModule::ShutdownModule()
{
	for (TUniquePtr<FGSBMenuExtenderBase>& MenuExtender : MenuExtenders)
	{
		MenuExtender->Uninitialize();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGoogleSheetsBridgeModule, GoogleSheetsBridge)