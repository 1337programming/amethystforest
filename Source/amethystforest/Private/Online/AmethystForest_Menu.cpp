// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystForest_Menu.h"
#include "AmethystMainMenu.h"
#include "AmethystWelcomeMenu.h"
#include "AmethystMessageMenu.h"
#include "AmethystPlayerController_Menu.h"
#include "Online/AmethystForestSession.h"

AAmethystForest_Menu::AAmethystForest_Menu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerControllerClass = AAmethystPlayerController_Menu::StaticClass();
}

void AAmethystForest_Menu::RestartPlayer(class AController* NewPlayer)
{
	// don't restart
}

/** Returns game session class to use */
TSubclassOf<AGameSession> AAmethystForest_Menu::GetGameSessionClass() const
{
	return AAmethystForestSession::StaticClass();
}
