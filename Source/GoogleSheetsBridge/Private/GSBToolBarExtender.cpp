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
	GSB::GenericRequest_GET(Asset);
}

void ExportToDefaultSpreadsheet(UObject* Asset)
{
	GSB::GenericRequest_POST(Asset);
}

void ExportToCustomSpreadsheet(UObject* Asset)
{
	GSB::CreateExportWindow(Asset);
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
					return GSB::IsSyncButtonExecutable(Asset);
				})),
				NAME_None,
				FText::FromString(GSB::Constants::SyncWithButtonName),
				FText::FromString(GSB::Constants::SyncWithTooltip),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Refresh")
			);

			ToolBarBuilder.AddToolBarButton(
				FUIAction(FExecuteAction::CreateLambda([Asset]
				{
					ExportToDefaultSpreadsheet(Asset);
				})),
				NAME_None,
				FText::FromString(GSB::Constants::ExportToDefaultButtonName),
				FText::FromString(GSB::Constants::ExportToDefaultTooltip),
				FSlateIcon()
			);

			ToolBarBuilder.AddToolBarButton(
				FUIAction(FExecuteAction::CreateLambda([Asset]
				{
					ExportToCustomSpreadsheet(Asset);
				})),
				NAME_None,
				FText::FromString(GSB::Constants::ExportToCustomButtonName),
				FText::FromString(GSB::Constants::ExportToCustomTooltip),
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
