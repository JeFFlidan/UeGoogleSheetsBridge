// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBMenuExtender.h"
#include "GoogleSheetsBridgeSettings.h"

#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"

void FGSBMenuExtender::Initialize()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& AssetMenuExtendersDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	AssetMenuExtendersDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FGSBMenuExtender::ExtendContextMenu));
	MenuExtenderDelegateHandle = AssetMenuExtendersDelegates.Last().GetHandle();
}

void FGSBMenuExtender::Uninitialize()
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

TSharedRef<FExtender> FGSBMenuExtender::ExtendContextMenu(const TArray<FAssetData>& AssetDataList)
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
		FMenuExtensionDelegate::CreateRaw(this, &FGSBMenuExtender::AddMenuEntries));

	return MenuExtender.ToSharedRef();
}

const FString& FGSBMenuExtender::GetSpreadsheetId(UObject* Object) const
{
	return GetDefault<UGoogleSheetsBridgeSettings>()->DefaultSpreadsheetId;
}
