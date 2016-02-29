// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Widgets/AmethystMenuItem.h"
#include "Widgets/SAmethystMenuWidget.h"
#include "AmethystOptions.h"
#include "AmethystFriends.h"
#include "AmethystRecentlyMet.h"

class FAmethystIngameMenu : public TSharedFromThis<FAmethystIngameMenu>
{
public:
	/** sets owning player controller */
	void Construct(ULocalPlayer* PlayerOwner);

	/** toggles in game menu */
	void ToggleGameMenu();

	/** is game menu currently active? */
	bool GetIsGameMenuUp() const;

	/* updates the friends list of the current owner*/
	void UpdateFriendsList();

	/* Getter for the AmethystFriends interface/pointer*/
	TSharedPtr<class FAmethystFriends> GetAmethystFriends(){ return AmethystFriends; }

protected:

	/** Owning player controller */
	ULocalPlayer* PlayerOwner;

	/** game menu container widget - used for removing */
	TSharedPtr<class SWeakWidget> GameMenuContainer;

	/** root menu item pointer */
	TSharedPtr<FAmethystMenuItem> RootMenuItem;

	/** main menu item pointer */
	TSharedPtr<FAmethystMenuItem> MainMenuItem;

	/** HUD menu widget */
	TSharedPtr<class SAmethystMenuWidget> GameMenuWidget;	

	/** if game menu is currently opened*/
	bool bIsGameMenuUp;

	/** holds cheats menu item to toggle it's visibility */
	TSharedPtr<class FAmethystMenuItem> CheatsMenu;

	/** Amethyst options */
	TSharedPtr<class FAmethystOptions> AmethystOptions;

	/** get current user index out of PlayerOwner */
	int32 GetOwnerUserIndex() const;
	/** Amethyst friends */
	TSharedPtr<class FAmethystFriends> AmethystFriends;

	/** Amethyst recently met users*/
	TSharedPtr<class FAmethystRecentlyMet> AmethystRecentlyMet;

	/** called when going back to previous menu */
	void OnMenuGoBack(MenuPtr Menu);
	
	/** goes back in menu structure */
	void CloseSubMenu();

	/** removes widget from viewport */
	void DetachGameMenu();
	
	/** Delegate called when user cancels confirmation dialog to exit to main menu */
	void OnCancelExitToMain();

	/** Delegate called when user confirms confirmation dialog to exit to main menu */
	void OnConfirmExitToMain();		

	/** Plays sound and calls Quit */
	void OnUIQuit();

	/** Quits the game */
	void Quit();

	/** Shows the system UI to invite friends to the game */
	void OnShowInviteUI();
};
