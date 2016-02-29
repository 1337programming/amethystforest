// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystPlayerState.h"

AAmethystPlayerState::AAmethystPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TeamNumber = 0;
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
	bQuitter = false;
}

void AAmethystPlayerState::Reset()
{
	Super::Reset();
	
	//PlayerStates persist across seamless travel.  Keep the same teams as previous match.
	//SetTeamNum(0);
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
	bQuitter = false;
}

void AAmethystPlayerState::UnregisterPlayerWithSession()
{
	if (!bFromPreviousLevel)
	{
		Super::UnregisterPlayerWithSession();
	}
}

void AAmethystPlayerState::ClientInitialize(AController* InController)
{
	Super::ClientInitialize(InController);

	UpdateTeamColors();
}

void AAmethystPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;

	UpdateTeamColors();
}

void AAmethystPlayerState::OnRep_TeamColor()
{
	UpdateTeamColors();
}

void AAmethystPlayerState::AddBulletsFired(int32 NumBullets)
{
	NumBulletsFired += NumBullets;
}

void AAmethystPlayerState::AddRocketsFired(int32 NumRockets)
{
	NumRocketsFired += NumRockets;
}

void AAmethystPlayerState::SetQuitter(bool bInQuitter)
{
	bQuitter = bInQuitter;
}

void AAmethystPlayerState::CopyProperties(APlayerState* PlayerState)
{	
	Super::CopyProperties(PlayerState);

	AAmethystPlayerState* AmethystPlayer = Cast<AAmethystPlayerState>(PlayerState);
	if (AmethystPlayer)
	{
		AmethystPlayer->TeamNumber = TeamNumber;
	}	
}

void AAmethystPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController != NULL)
	{
		AAmethystCharacter* AmethystCharacter = Cast<AAmethystCharacter>(OwnerController->GetCharacter());
		if (AmethystCharacter != NULL)
		{
			AmethystCharacter->UpdateTeamColorsAllMIDs();
		}
	}
}

int32 AAmethystPlayerState::GetTeamNum() const
{
	return TeamNumber;
}

int32 AAmethystPlayerState::GetKills() const
{
	return NumKills;
}

int32 AAmethystPlayerState::GetDeaths() const
{
	return NumDeaths;
}

float AAmethystPlayerState::GetScore() const
{
	return Score;
}

int32 AAmethystPlayerState::GetNumBulletsFired() const
{
	return NumBulletsFired;
}

int32 AAmethystPlayerState::GetNumRocketsFired() const
{
	return NumRocketsFired;
}

bool AAmethystPlayerState::IsQuitter() const
{
	return bQuitter;
}

void AAmethystPlayerState::ScoreKill(AAmethystPlayerState* Victim, int32 Points)
{
	NumKills++;
	ScorePoints(Points);
}

void AAmethystPlayerState::ScoreDeath(AAmethystPlayerState* KilledBy, int32 Points)
{
	NumDeaths++;
	ScorePoints(Points);
}

void AAmethystPlayerState::ScorePoints(int32 Points)
{
	AAmethystForestState* const MyGameState = Cast<AAmethystForestState>(GetWorld()->GameState);
	if (MyGameState && TeamNumber >= 0)
	{
		if (TeamNumber >= MyGameState->TeamScores.Num())
		{
			MyGameState->TeamScores.AddZeroed(TeamNumber - MyGameState->TeamScores.Num() + 1);
		}

		MyGameState->TeamScores[TeamNumber] += Points;
	}

	Score += Points;
}

void AAmethystPlayerState::InformAboutKill_Implementation(class AAmethystPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AAmethystPlayerState* KilledPlayerState)
{
	//id can be null for bots
	if (KillerPlayerState->UniqueId.IsValid())
	{	
		//search for the actual killer before calling OnKill()	
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{		
			AAmethystPlayerController* TestPC = Cast<AAmethystPlayerController>(*It);
			if (TestPC && TestPC->IsLocalController())
			{
				// a local player might not have an ID if it was created with CreateDebugPlayer.
				ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(TestPC->Player);
				TSharedPtr<const FUniqueNetId> LocalID = LocalPlayer->GetCachedUniqueNetId();
				if (LocalID.IsValid() &&  *LocalPlayer->GetCachedUniqueNetId() == *KillerPlayerState->UniqueId)
				{			
					TestPC->OnKill();
				}
			}
		}
	}
}

void AAmethystPlayerState::BroadcastDeath_Implementation(class AAmethystPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AAmethystPlayerState* KilledPlayerState)
{	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// all local players get death messages so they can update their huds.
		AAmethystPlayerController* TestPC = Cast<AAmethystPlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			TestPC->OnDeathMessage(KillerPlayerState, this, KillerDamageType);				
		}
	}	
}

void AAmethystPlayerState::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( AAmethystPlayerState, TeamNumber );
	DOREPLIFETIME( AAmethystPlayerState, NumKills );
	DOREPLIFETIME( AAmethystPlayerState, NumDeaths );
}

FString AAmethystPlayerState::GetShortPlayerName() const
{
	if( PlayerName.Len() > MAX_PLAYER_NAME_LENGTH )
	{
		return PlayerName.Left(MAX_PLAYER_NAME_LENGTH) + "...";
	}
	return PlayerName;
}
