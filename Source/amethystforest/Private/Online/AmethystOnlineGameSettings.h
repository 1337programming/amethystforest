// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * General session settings for a Amethyst game
 */
class FAmethystOnlineSessionSettings : public FOnlineSessionSettings
{
public:

	FAmethystOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);
	virtual ~FAmethystOnlineSessionSettings() {}
};

/**
 * General search setting for a Amethyst game
 */
class FAmethystOnlineSearchSettings : public FOnlineSessionSearch
{
public:
	FAmethystOnlineSearchSettings(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FAmethystOnlineSearchSettings() {}
};

/**
 * Search settings for an empty dedicated server to host a match
 */
class FAmethystOnlineSearchSettingsEmptyDedicated : public FAmethystOnlineSearchSettings
{
public:
	FAmethystOnlineSearchSettingsEmptyDedicated(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FAmethystOnlineSearchSettingsEmptyDedicated() {}
};
