// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AmethystBot.generated.h"

UCLASS()
class AAmethystBot : public AAmethystCharacter
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category=Behavior)
	class UBehaviorTree* BotBehavior;

	virtual bool IsFirstPerson() const override;

	virtual void FaceRotation(FRotator NewRotation, float DeltaTime = 0.f) override;
};