// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBMenuExtender.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FGoogleSheetsBridgeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TArray<TUniquePtr<FGSBMenuExtenderBase>> MenuExtenders;
};
