// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "Player/AmethystCheatManager.h"
#include "Online/AmethystPlayerState.h"
#include "Bots/AmethystAIController.h"

UAmethystCheatManager::UAmethystCheatManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UAmethystCheatManager::ToggleInfiniteAmmo()
{
	AAmethystPlayerController* MyPC = GetOuterAAmethystPlayerController();

	MyPC->SetInfiniteAmmo(!MyPC->HasInfiniteAmmo());
	MyPC->ClientMessage(FString::Printf(TEXT("Infinite ammo: %s"), MyPC->HasInfiniteAmmo() ? TEXT("ENABLED") : TEXT("off")));
}

void UAmethystCheatManager::ToggleInfiniteClip()
{
	AAmethystPlayerController* MyPC = GetOuterAAmethystPlayerController();

	MyPC->SetInfiniteClip(!MyPC->HasInfiniteClip());
	MyPC->ClientMessage(FString::Printf(TEXT("Infinite clip: %s"), MyPC->HasInfiniteClip() ? TEXT("ENABLED") : TEXT("off")));
}

void UAmethystCheatManager::ToggleMatchTimer()
{
	AAmethystPlayerController* MyPC = GetOuterAAmethystPlayerController();

	AAmethystForestState* const MyGameState = Cast<AAmethystForestState>(MyPC->GetWorld()->GameState);
	if (MyGameState && MyGameState->Role == ROLE_Authority)
	{
		MyGameState->bTimerPaused = !MyGameState->bTimerPaused;
		MyPC->ClientMessage(FString::Printf(TEXT("Match timer: %s"), MyGameState->bTimerPaused ? TEXT("PAUSED") : TEXT("running")));
	}
}

void UAmethystCheatManager::ForceMatchStart()
{
	AAmethystPlayerController* const MyPC = GetOuterAAmethystPlayerController();

	AAmethystForestMode* const MyGame = MyPC->GetWorld()->GetAuthGameMode<AAmethystForestMode>();
	if (MyGame && MyGame->GetMatchState() == MatchState::WaitingToStart)
	{
		MyGame->StartMatch();
	}
}

void UAmethystCheatManager::ChangeTeam(int32 NewTeamNumber)
{
	AAmethystPlayerController* MyPC = GetOuterAAmethystPlayerController();

	AAmethystPlayerState* MyPlayerState = Cast<AAmethystPlayerState>(MyPC->PlayerState);
	if (MyPlayerState && MyPlayerState->Role == ROLE_Authority)
	{
		MyPlayerState->SetTeamNum(NewTeamNumber);
		MyPC->ClientMessage(FString::Printf(TEXT("Team changed to: %d"), MyPlayerState->GetTeamNum()));
	}
}

void UAmethystCheatManager::Cheat(const FString& Msg)
{
	GetOuterAAmethystPlayerController()->ServerCheat(Msg.Left(128));
}

void UAmethystCheatManager::SpawnBot()
{
	AAmethystPlayerController* const MyPC = GetOuterAAmethystPlayerController();
	APawn* const MyPawn = MyPC->GetPawn();
	AAmethystForestMode* const MyGame = MyPC->GetWorld()->GetAuthGameMode<AAmethystForestMode>();
	UWorld* World = MyPC->GetWorld();
	if (MyPawn && MyGame && World)
	{
		static int32 CheatBotNum = 50;
		AAmethystAIController* AmethystAIController = MyGame->CreateBot(CheatBotNum++);
		MyGame->RestartPlayer(AmethystAIController);
	}
}