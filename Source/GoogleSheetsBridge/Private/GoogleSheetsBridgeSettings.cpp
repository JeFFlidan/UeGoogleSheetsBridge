// Copyright Kyrylo Zaverukha. All Rights Reserved.


#include "GoogleSheetsBridgeSettings.h"

UGoogleSheetsBridgeSettings::UGoogleSheetsBridgeSettings(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	CategoryName = "Plugins";
	SectionName = "Google Sheets Bridge";
	
	ServerPort = 8080;
	ApiScriptId = "AKfycbxl31lGa1EHUJm0bOyvsSdLq2vNQiURaf46izk7nxWx1-Hz8p1gE_WAtRHvTEoxyMuw";
}
