// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FGSBMenuExtenderBase;

class FGoogleSheetsBridgeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TArray<TUniquePtr<FGSBMenuExtenderBase>> MenuExtenders;

	void SyncAssetsWithGoogleSheets(UClass* Class);
};
