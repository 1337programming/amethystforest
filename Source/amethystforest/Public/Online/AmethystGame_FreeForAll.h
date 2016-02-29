// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AmethystForest_FreeForAll.generated.h"

class AAmethystPlayerState;

UCLASS()
class AAmethystForest_FreeForAll : public AAmethystForestMode
{
	GENERATED_UCLASS_BODY()

protected:

	/** best player */
	UPROPERTY(transient)
	AAmethystPlayerState* WinnerPlayerState;

	/** check who won */
	virtual void DetermineMatchWinner() override;

	/** check if PlayerState is a winner */
	virtual bool IsWinner(AAmethystPlayerState* PlayerState) const override;
};
