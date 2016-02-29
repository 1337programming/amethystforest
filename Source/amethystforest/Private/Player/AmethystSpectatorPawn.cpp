// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "AmethystForest.h"
#include "Player/AmethystSpectatorPawn.h"

AAmethystSpectatorPawn::AAmethystSpectatorPawn(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

void AAmethystSpectatorPawn::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	check(InputComponent);
	
	InputComponent->BindAxis("MoveForward", this, &ADefaultPawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ADefaultPawn::MoveRight);
	InputComponent->BindAxis("MoveUp", this, &ADefaultPawn::MoveUp_World);
	InputComponent->BindAxis("Turn", this, &ADefaultPawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ADefaultPawn::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ADefaultPawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AAmethystSpectatorPawn::LookUpAtRate);
}

void AAmethystSpectatorPawn::LookUpAtRate(float Val)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Val * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}
