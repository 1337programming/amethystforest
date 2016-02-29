// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AmethystPickup.h"
#include "AmethystPickup_Health.generated.h"

class AAmethystCharacter;

// A pickup object that replenishes character health
UCLASS(Abstract, Blueprintable)
class AAmethystPickup_Health : public AAmethystPickup
{
	GENERATED_UCLASS_BODY()

	/** check if pawn can use this pickup */
	virtual bool CanBePickedUp(AAmethystCharacter* TestPawn) const override;

protected:

	/** how much health does it give? */
	UPROPERTY(EditDefaultsOnly, Category=Pickup)
	int32 Health;

	/** give pickup */
	virtual void GivePickupTo(AAmethystCharacter* Pawn) override;
};
