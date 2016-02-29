// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystForestDelegates.h"

#include "AmethystMenuSoundsWidgetStyle.h"
#include "AmethystMenuWidgetStyle.h"
#include "AmethystMenuItemWidgetStyle.h"
#include "AmethystOptionsWidgetStyle.h"
#include "AmethystScoreboardWidgetStyle.h"
#include "AmethystChatWidgetStyle.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"



#include "UI/Style/AmethystStyle.h"


class FAmethystForestModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
		InitializeAmethystForestDelegates();
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		//Hot reload hack
		FSlateStyleRegistry::UnRegisterSlateStyle(FAmethystStyle::GetStyleSetName());
		FAmethystStyle::Initialize();
	}

	virtual void ShutdownModule() override
	{
		FAmethystStyle::Shutdown();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FAmethystForestModule, AmethystForest, "AmethystForest");

DEFINE_LOG_CATEGORY(LogAmethyst)
DEFINE_LOG_CATEGORY(LogAmethystWeapon)
