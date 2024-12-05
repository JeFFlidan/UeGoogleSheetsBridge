// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GSBAsset.h"
#include "Widgets/SCompoundWidget.h"

class SGSBExportSpreadsheetWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SGSBExportSpreadsheetWidget)
		{}
		SLATE_ARGUMENT(TSharedPtr<SWindow>, Window)
		SLATE_ARGUMENT(FGSBAsset, Asset)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
};
