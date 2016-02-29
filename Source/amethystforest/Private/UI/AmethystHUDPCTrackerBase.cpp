// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystHUDPCTrackerBase.h"


/** Initialize with a world context. */
void AmethystHUDPCTrackerBase::Init( const FLocalPlayerContext& InContext )
{
	Context = InContext;
}

TWeakObjectPtr<AAmethystPlayerController> AmethystHUDPCTrackerBase::GetPlayerController() const
{
	if ( ensureMsgf( Context.IsValid(), TEXT("Game context must be initialized!") ) )
	{
		APlayerController* PC = Context.GetPlayerController();
		AAmethystPlayerController* AmethystPC = Cast<AAmethystPlayerController>(PC);
		return TWeakObjectPtr<AAmethystPlayerController>(AmethystPC);
	}
	else
	{
		return NULL;
	}
}


UWorld* AmethystHUDPCTrackerBase::GetWorld() const
{
	if ( ensureMsgf( Context.IsValid(), TEXT("Game context must be initialized!") ) )
	{
		return Context.GetWorld();
	}
	else
	{
		return NULL;
	}
}

AAmethystForestState* AmethystHUDPCTrackerBase::GetGameState() const
{
	if ( ensureMsgf( Context.IsValid(), TEXT("Game context must be initialized!") ) )
	{
		return Cast<AAmethystForestState>(Context.GetWorld()->GameState);
	}
	else
	{
		return NULL;
	}
}

const FLocalPlayerContext& AmethystHUDPCTrackerBase::GetContext() const
{
	return Context;
}



