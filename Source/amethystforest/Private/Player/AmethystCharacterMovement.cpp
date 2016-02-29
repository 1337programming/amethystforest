// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "Player/AmethystCharacterMovement.h"

//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UAmethystCharacterMovement::UAmethystCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


float UAmethystCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AAmethystCharacter* AmethystCharacterOwner = Cast<AAmethystCharacter>(PawnOwner);
	if (AmethystCharacterOwner)
	{
		if (AmethystCharacterOwner->IsTargeting())
		{
			MaxSpeed *= AmethystCharacterOwner->GetTargetingSpeedModifier();
		}
		if (AmethystCharacterOwner->IsRunning())
		{
			MaxSpeed *= AmethystCharacterOwner->GetRunningSpeedModifier();
		}
	}

	return MaxSpeed;
}
