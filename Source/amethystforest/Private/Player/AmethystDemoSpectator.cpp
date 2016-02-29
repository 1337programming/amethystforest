// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "Player/AmethystDemoSpectator.h"
#include "UI/Menu/AmethystDemoPlaybackMenu.h"
#include "UI/Widgets/SAmethystDemoHUD.h"
#include "Engine/DemoNetDriver.h"

AAmethystDemoSpectator::AAmethystDemoSpectator(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bShowMouseCursor = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;
}

void AAmethystDemoSpectator::SetupInputComponent()
{
	Super::SetupInputComponent();

	// UI input
	InputComponent->BindAction( "InGameMenu", IE_Pressed, this, &AAmethystDemoSpectator::OnToggleInGameMenu );

	InputComponent->BindAction( "NextWeapon", IE_Pressed, this, &AAmethystDemoSpectator::OnIncreasePlaybackSpeed );
	InputComponent->BindAction( "PrevWeapon", IE_Pressed, this, &AAmethystDemoSpectator::OnDecreasePlaybackSpeed );
}

void AAmethystDemoSpectator::SetPlayer( UPlayer* InPlayer )
{
	Super::SetPlayer( InPlayer );

	// Build menu only after game is initialized
	AmethystDemoPlaybackMenu = MakeShareable( new FAmethystDemoPlaybackMenu() );
	AmethystDemoPlaybackMenu->Construct( Cast< ULocalPlayer >( Player ) );

	// Create HUD if this is playback
	if (GetWorld() != nullptr && GetWorld()->DemoNetDriver != nullptr && !GetWorld()->DemoNetDriver->IsServer())
	{
		if (GEngine != nullptr && GEngine->GameViewport != nullptr)
		{
			DemoHUD = SNew(SAmethystDemoHUD)
				.PlayerOwner(this);

			GEngine->GameViewport->AddViewportWidgetContent(DemoHUD.ToSharedRef());
		}
	}

	FActorSpawnParameters SpawnInfo;

	SpawnInfo.Owner				= this;
	SpawnInfo.Instigator		= Instigator;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PlaybackSpeed = 2;

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(DemoHUD);

	SetInputMode(InputMode);
}

void AAmethystDemoSpectator::OnToggleInGameMenu()
{
	// if no one's paused, pause
	if ( AmethystDemoPlaybackMenu.IsValid() )
	{
		AmethystDemoPlaybackMenu->ToggleGameMenu();
	}
}

static float PlaybackSpeedLUT[5] = { 0.1f, 0.5f, 1.0f, 2.0f, 4.0f };

void AAmethystDemoSpectator::OnIncreasePlaybackSpeed()
{
	PlaybackSpeed = FMath::Clamp( PlaybackSpeed + 1, 0, 4 );

	GetWorldSettings()->DemoPlayTimeDilation = PlaybackSpeedLUT[ PlaybackSpeed ];
}

void AAmethystDemoSpectator::OnDecreasePlaybackSpeed()
{
	PlaybackSpeed = FMath::Clamp( PlaybackSpeed - 1, 0, 4 );

	GetWorldSettings()->DemoPlayTimeDilation = PlaybackSpeedLUT[ PlaybackSpeed ];
}

void AAmethystDemoSpectator::Destroyed()
{
	if (GEngine != nullptr && GEngine->GameViewport != nullptr && DemoHUD.IsValid())
	{
		// Remove HUD
		GEngine->GameViewport->RemoveViewportWidgetContent(DemoHUD.ToSharedRef());
	}

	Super::Destroyed();
}
