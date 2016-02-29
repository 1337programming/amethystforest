// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystIngameMenu.h"
#include "AmethystStyle.h"
#include "AmethystMenuSoundsWidgetStyle.h"
#include "Online.h"
#include "OnlineExternalUIInterface.h"
#include "AmethystForestInstance.h"
#include "UI/AmethystHUD.h"

#define LOCTEXT_NAMESPACE "AmethystForest.HUD.Menu"

void FAmethystIngameMenu::Construct(ULocalPlayer* _PlayerOwner)
{
	PlayerOwner = _PlayerOwner;
	bIsGameMenuUp = false;

	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}
	
	//todo:  don't create ingame menus for remote players.
	const UAmethystForestInstance* GameInstance = nullptr;
	if (PlayerOwner)
	{
		GameInstance = Cast<UAmethystForestInstance>(PlayerOwner->GetGameInstance());
	}

	if (!GameMenuWidget.IsValid())
	{
		SAssignNew(GameMenuWidget, SAmethystMenuWidget)
			.PlayerOwner(TWeakObjectPtr<ULocalPlayer>(PlayerOwner))
			.Cursor(EMouseCursor::Default)
			.IsGameMenu(true);			


		int32 const OwnerUserIndex = GetOwnerUserIndex();

		// setup the exit to main menu submenu.  We wanted a confirmation to avoid a potential TRC violation.
		// fixes TTP: 322267
		TSharedPtr<FAmethystMenuItem> MainMenuRoot = FAmethystMenuItem::CreateRoot();
		MainMenuItem = MenuHelper::AddMenuItem(MainMenuRoot,LOCTEXT("Main Menu", "MAIN MENU"));
		MenuHelper::AddMenuItemSP(MainMenuItem,LOCTEXT("No", "NO"), this, &FAmethystIngameMenu::OnCancelExitToMain);
		MenuHelper::AddMenuItemSP(MainMenuItem,LOCTEXT("Yes", "YES"), this, &FAmethystIngameMenu::OnConfirmExitToMain);

		AmethystOptions = MakeShareable(new FAmethystOptions());
		AmethystOptions->Construct(PlayerOwner);
		AmethystOptions->TellInputAboutKeybindings();
		AmethystOptions->OnApplyChanges.BindSP(this, &FAmethystIngameMenu::CloseSubMenu);

		MenuHelper::AddExistingMenuItem(RootMenuItem, AmethystOptions->CheatsItem.ToSharedRef());
		MenuHelper::AddExistingMenuItem(RootMenuItem, AmethystOptions->OptionsItem.ToSharedRef());
		if (GameInstance && GameInstance->GetIsOnline())
		{
#if !PLATFORM_XBOXONE
			AmethystFriends = MakeShareable(new FAmethystFriends());
			AmethystFriends->Construct(PlayerOwner, OwnerUserIndex);
			AmethystFriends->TellInputAboutKeybindings();
			AmethystFriends->OnApplyChanges.BindSP(this, &FAmethystIngameMenu::CloseSubMenu);

			MenuHelper::AddExistingMenuItem(RootMenuItem, AmethystFriends->FriendsItem.ToSharedRef());

			AmethystRecentlyMet = MakeShareable(new FAmethystRecentlyMet());
			AmethystRecentlyMet->Construct(PlayerOwner, OwnerUserIndex);
			AmethystRecentlyMet->TellInputAboutKeybindings();
			AmethystRecentlyMet->OnApplyChanges.BindSP(this, &FAmethystIngameMenu::CloseSubMenu);

			MenuHelper::AddExistingMenuItem(RootMenuItem, AmethystRecentlyMet->RecentlyMetItem.ToSharedRef());
#endif		

#if SHOOTER_CONSOLE_UI			
			TSharedPtr<FAmethystMenuItem> ShowInvitesItem = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Invite Players", "INVITE PLAYERS"));
			ShowInvitesItem->OnConfirmMenuItem.BindRaw(this, &FAmethystIngameMenu::OnShowInviteUI);
#endif
		}

		if (FSlateApplication::Get().SupportsSystemHelp())
		{
			TSharedPtr<FAmethystMenuItem> HelpSubMenu = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Help", "HELP"));
			HelpSubMenu->OnConfirmMenuItem.BindStatic([](){ FSlateApplication::Get().ShowSystemHelp(); });
		}

		MenuHelper::AddExistingMenuItem(RootMenuItem, MainMenuItem.ToSharedRef());
				
#if !SHOOTER_CONSOLE_UI
		MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("Quit", "QUIT"), this, &FAmethystIngameMenu::OnUIQuit);
#endif

		GameMenuWidget->MainMenu = GameMenuWidget->CurrentMenu = RootMenuItem->SubMenu;
		GameMenuWidget->OnMenuHidden.BindSP(this,&FAmethystIngameMenu::DetachGameMenu);
		GameMenuWidget->OnToggleMenu.BindSP(this,&FAmethystIngameMenu::ToggleGameMenu);
		GameMenuWidget->OnGoBack.BindSP(this, &FAmethystIngameMenu::OnMenuGoBack);
	}
}

void FAmethystIngameMenu::CloseSubMenu()
{
	GameMenuWidget->MenuGoBack();
}

void FAmethystIngameMenu::OnMenuGoBack(MenuPtr Menu)
{
	// if we are going back from options menu
	if (AmethystOptions.IsValid() && AmethystOptions->OptionsItem->SubMenu == Menu)
	{
		AmethystOptions->RevertChanges();
	}
}

bool FAmethystIngameMenu::GetIsGameMenuUp() const
{
	return bIsGameMenuUp;
}

void FAmethystIngameMenu::UpdateFriendsList()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineFriendsPtr OnlineFriendsPtr = OnlineSub->GetFriendsInterface();
		if (OnlineFriendsPtr.IsValid())
		{
			OnlineFriendsPtr->ReadFriendsList(GetOwnerUserIndex(), EFriendsLists::ToString(EFriendsLists::OnlinePlayers));
		}
	}
}

void FAmethystIngameMenu::DetachGameMenu()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(GameMenuContainer.ToSharedRef());
	}
	bIsGameMenuUp = false;

	AAmethystPlayerController* const PCOwner = PlayerOwner ? Cast<AAmethystPlayerController>(PlayerOwner->PlayerController) : nullptr;
	if (PCOwner)
	{
		PCOwner->SetPause(false);

		// If the game is over enable the scoreboard
		AAmethystHUD* const AmethystHUD = PCOwner->GetAmethystHUD();
		if( ( AmethystHUD != NULL ) && ( AmethystHUD->IsMatchOver() == true ) && ( PCOwner->IsPrimaryPlayer() == true ) )
		{
			AmethystHUD->ShowScoreboard( true, true );
		}
	}
}

void FAmethystIngameMenu::ToggleGameMenu()
{
	//Update the owner in case the menu was opened by another controller
	//UpdateMenuOwner();

	if (!GameMenuWidget.IsValid())
	{
		return;
	}

	// check for a valid user index.  could be invalid if the user signed out, in which case the 'please connect your control' ui should be up anyway.
	// in-game menu needs a valid userindex for many OSS calls.
	if (GetOwnerUserIndex() == -1)
	{
		UE_LOG(LogAmethyst, Log, TEXT("Trying to toggle in-game menu for invalid userid"));
		return;
	}

	if (bIsGameMenuUp && GameMenuWidget->CurrentMenu != RootMenuItem->SubMenu)
	{
		GameMenuWidget->MenuGoBack();
		return;
	}
	
	AAmethystPlayerController* const PCOwner = PlayerOwner ? Cast<AAmethystPlayerController>(PlayerOwner->PlayerController) : nullptr;
	if (!bIsGameMenuUp)
	{
		// Hide the scoreboard
		if (PCOwner)
		{
			AAmethystHUD* const AmethystHUD = PCOwner->GetAmethystHUD();
			if( AmethystHUD != NULL )
			{
				AmethystHUD->ShowScoreboard( false );
			}
		}

		GEngine->GameViewport->AddViewportWidgetContent(
			SAssignNew(GameMenuContainer,SWeakWidget)
			.PossiblyNullContent(GameMenuWidget.ToSharedRef())
			);

		int32 const OwnerUserIndex = GetOwnerUserIndex();
		if(AmethystOptions.IsValid())
		{
			AmethystOptions->UpdateOptions();
		}
		if(AmethystRecentlyMet.IsValid())
		{
			AmethystRecentlyMet->UpdateRecentlyMet(OwnerUserIndex);
		}
		GameMenuWidget->BuildAndShowMenu();
		bIsGameMenuUp = true;

		if (PCOwner)
		{
			// Disable controls while paused
			PCOwner->SetCinematicMode(true, false, false, true, true);

			PCOwner->SetPause(true);
		}
	} 
	else
	{
		//Start hiding animation
		GameMenuWidget->HideMenu();
		if (PCOwner)
		{
			// Make sure viewport has focus
			FSlateApplication::Get().SetAllUserFocusToGameViewport();

			// Don't renable controls if the match is over
			AAmethystHUD* const AmethystHUD = PCOwner->GetAmethystHUD();
			if( ( AmethystHUD != NULL ) && ( AmethystHUD->IsMatchOver() == false ) )
			{
				PCOwner->SetCinematicMode(false,false,false,true,true);
			}
		}
	}
}

void FAmethystIngameMenu::OnCancelExitToMain()
{
	CloseSubMenu();
}

void FAmethystIngameMenu::OnConfirmExitToMain()
{
	UAmethystForestInstance* const GameInstance = Cast<UAmethystForestInstance>(PlayerOwner->GetGameInstance());
	if (GameInstance)
	{
		GameInstance->LabelPlayerAsQuitter(PlayerOwner);

		// tell game instance to go back to main menu state
		GameInstance->GotoState(AmethystForestInstanceState::MainMenu);
	}
}

void FAmethystIngameMenu::OnUIQuit()
{
	UAmethystForestInstance* const GI = Cast<UAmethystForestInstance>(PlayerOwner->GetGameInstance());
	if (GI)
	{
		GI->LabelPlayerAsQuitter(PlayerOwner);
	}

	GameMenuWidget->LockControls(true);
	GameMenuWidget->HideMenu();

	UWorld* const World = PlayerOwner ? PlayerOwner->GetWorld() : nullptr;
	if (World)
	{
		const FAmethystMenuSoundsStyle& MenuSounds = FAmethystStyle::Get().GetWidgetStyle<FAmethystMenuSoundsStyle>("DefaultAmethystMenuSoundsStyle");
		MenuHelper::PlaySoundAndCall(World, MenuSounds.ExitGameSound, GetOwnerUserIndex(), this, &FAmethystIngameMenu::Quit);
	}
}

void FAmethystIngameMenu::Quit()
{
	APlayerController* const PCOwner = PlayerOwner ? PlayerOwner->PlayerController : nullptr;
	if (PCOwner)
	{
		PCOwner->ConsoleCommand("quit");
	}
}

void FAmethystIngameMenu::OnShowInviteUI()
{
	const auto ExternalUI = Online::GetExternalUIInterface();

	if(!ExternalUI.IsValid())
	{
		UE_LOG(LogAmethyst, Warning, TEXT("OnShowInviteUI: External UI interface is not supported on this platform."));
		return;
	}

	ExternalUI->ShowInviteUI(GetOwnerUserIndex());
}

int32 FAmethystIngameMenu::GetOwnerUserIndex() const
{
	return PlayerOwner ? PlayerOwner->GetControllerId() : 0;
}


#undef LOCTEXT_NAMESPACE
