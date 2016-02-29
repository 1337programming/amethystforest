// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AmethystPersistentUser.h"
#include "AmethystLocalPlayer.generated.h"

UCLASS(config=Engine, transient)
class UAmethystLocalPlayer : public ULocalPlayer
{
	GENERATED_UCLASS_BODY()

public:

	virtual void SetControllerId(int32 NewControllerId) override;

	virtual FString GetNickname() const;

	class UAmethystPersistentUser* GetPersistentUser() const;
	
	/** Initializes the PersistentUser */
	void LoadPersistentUser();

private:
	/** Persistent user data stored between sessions (i.e. the user's savegame) */
	UPROPERTY()
	class UAmethystPersistentUser* PersistentUser;
};



