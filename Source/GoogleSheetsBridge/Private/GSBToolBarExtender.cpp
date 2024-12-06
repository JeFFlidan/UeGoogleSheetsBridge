// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "GSBToolBarExtender.h"
#include "GoogleSheetsBridgeLogChannels.h"
#include "GSBUtils.h"
#include "GSBConstants.h"

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
	FGSBUtils::GenericRequest_GET(Asset);
}

void ExportToDefaultSpreadsheet(UObject* Asset)
{
	FGSBUtils::GenericRequest_POST(Asset);
}

void ExportToCustomSpreadsheet(UObject* Asset)
{
	FGSBUtils::CreateExportWindow(Asset);
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
			ToolBarBuilder.AddSeparator();
			ToolBarBuilder.AddToolBarButton(
				FUIAction(FExecuteAction::CreateLambda([Asset]
				{
					SyncWithGoogleSheets(Asset);
				}), FCanExecuteAction::CreateLambda([Asset]()->bool
				{
					return FGSBUtils::IsSyncButtonExecutable(Asset);
				})),
				NAME_None,
				FText::FromString(GSB::Constants::SyncWithSpreadsheetButtonName),
				FText::FromString(GSB::Constants::SyncWithSpreadsheetTooltip),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Refresh")
			);

			ToolBarBuilder.AddToolBarButton(
				FUIAction(FExecuteAction::CreateLambda([Asset]
				{
					ExportToDefaultSpreadsheet(Asset);
				})),
				NAME_None,
				FText::FromString(GSB::Constants::ExportToSpreadsheetButtonName),
				FText::FromString(GSB::Constants::ExportToSpreadsheetTooltip),
				FSlateIcon()
			);

			ToolBarBuilder.AddToolBarButton(
				FUIAction(FExecuteAction::CreateLambda([Asset]
				{
					ExportToCustomSpreadsheet(Asset);
				})),
				NAME_None,
				FText::FromString(GSB::Constants::ChangeSpreadsheetIDButtonName),
				FText::FromString(GSB::Constants::ChangeSpreadsheetIDTooltip),
				FSlateIcon()
			);
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
