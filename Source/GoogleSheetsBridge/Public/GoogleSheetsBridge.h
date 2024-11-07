// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FGoogleSheetsBridgeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FDelegateHandle AssetMenuExtenderDelegateHandle;
	UDataAsset* SelectedDataAsset{nullptr};
	
	void AddMenuEntry(FMenuBuilder& MenuBuilder);
	void OpenExplorerToSave();
	
	TSharedRef<FExtender> DataAssetContextMenuExtender(const TArray<FAssetData>& AssetDataList);
};
