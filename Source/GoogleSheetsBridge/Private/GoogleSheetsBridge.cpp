// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GoogleSheetsBridge.h"
#include "GSBMenuExtender_DataTable.h"
#include "GSBMenuExtender_DataAsset.h"
#include "GSBMenuExtender_CurveTable.h"

#define LOCTEXT_NAMESPACE "FGoogleSheetsBridgeModule"

void FGoogleSheetsBridgeModule::StartupModule()
{
	MenuExtenders.Push(MakeUnique<FGSBMenuExtender_DataAsset>());
	
	for (TUniquePtr<FGSBMenuExtender>& MenuExtender : MenuExtenders)
	{
		MenuExtender->Initialize();
	}
}

void FGoogleSheetsBridgeModule::ShutdownModule()
{
	for (TUniquePtr<FGSBMenuExtender>& MenuExtender : MenuExtenders)
	{
		MenuExtender->Uninitialize();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGoogleSheetsBridgeModule, GoogleSheetsBridge)