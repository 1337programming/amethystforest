// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "Player/AmethystPlayerController_Menu.h"
#include "AmethystStyle.h"


AAmethystPlayerController_Menu::AAmethystPlayerController_Menu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void AAmethystPlayerController_Menu::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FAmethystStyle::Initialize();
}
