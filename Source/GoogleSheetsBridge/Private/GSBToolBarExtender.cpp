// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBToolBarExtender.h"
#include "GoogleSheetsApi.h"
#include "GoogleSheetsBridgeLogChannels.h"

#include "Editor.h"

UAssetEditorSubsystem* GetAssetEditorSubsystem()
{
	return GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
}

bool IsAssetClassValid(UClass* AssetClass)
{
	return AssetClass->IsChildOf(UDataAsset::StaticClass())
		|| AssetClass->IsChildOf(UDataTable::StaticClass())
		|| AssetClass->IsChildOf(UCurveTable::StaticClass());
}

void SyncWithGoogleSheets(UObject* Asset)
{
	FGoogleSheetsApiParams_GET Params(Asset);
	FGoogleSheetsApi::SendRequest_GET(Params, FGoogleSheetsApi::OnResponse_GET(Asset));
}

TSharedRef<FExtender> GetToolBarExtender(UObject* Asset)
{
	TSharedRef<FExtender> ToolBarExtender = MakeShared<FExtender>();
	ToolBarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateLambda([Asset](FToolBarBuilder& ToolBarBuilder)
		{
			ToolBarBuilder.AddToolBarButton(
				FUIAction(FExecuteAction::CreateLambda([Asset]
				{
					SyncWithGoogleSheets(Asset);
				})),
				NAME_None,
				FText::FromString("Sync with Google Sheets"),
				FText::FromString("Sync with Google Sheets"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Refresh"));
		})
	);

	return ToolBarExtender;
}

void HandleAssetEditorOpened(UObject* Asset)
{
	if (!IsAssetClassValid(Asset->GetClass()))
	{
		return;
	}
	
	IAssetEditorInstance* AssetEditorInstance = GetAssetEditorSubsystem()->FindEditorForAsset(Asset, false);
	if (!AssetEditorInstance)
	{
		UE_LOG(LogGoogleSheetsBridge, Warning, TEXT("Failed to find editor instance for %s"),
			*Asset->GetFName().ToString());
		return;
	}

	FAssetEditorToolkit* AssetEditorToolkit = static_cast<FAssetEditorToolkit*>(AssetEditorInstance);
	if (!AssetEditorToolkit)
	{
		UE_LOG(LogGoogleSheetsBridge, Warning, TEXT("Failed to cast IAssetEditorInstance to FAssetEditorToolkit"));
		return;
	}

	AssetEditorToolkit->AddToolbarExtender(GetToolBarExtender(Asset));
	AssetEditorToolkit->RegenerateMenusAndToolbars();
}

void FGSBToolBarExtender::Initialize()
{
	GetAssetEditorSubsystem()->OnAssetEditorOpened().AddStatic(&HandleAssetEditorOpened);
}
