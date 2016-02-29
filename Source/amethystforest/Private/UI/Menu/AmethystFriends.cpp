// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystFriends.h"
#include "AmethystTypes.h"
#include "AmethystStyle.h"
#include "AmethystOptionsWidgetStyle.h"
#include "Player/AmethystPersistentUser.h"
#include "AmethystForestUserSettings.h"
#include "AmethystLocalPlayer.h"

#define LOCTEXT_NAMESPACE "AmethystForest.HUD.Menu"

void FAmethystFriends::Construct(ULocalPlayer* _PlayerOwner, int32 LocalUserNum_)
{
	FriendsStyle = &FAmethystStyle::Get().GetWidgetStyle<FAmethystOptionsStyle>("DefaultAmethystOptionsStyle");

	PlayerOwner = _PlayerOwner;
	LocalUserNum = LocalUserNum_;
	CurrFriendIndex = 0;
	MinFriendIndex = 0;
	MaxFriendIndex = 0; //initialized after the friends list is read in

	/** Friends menu root item */
	TSharedPtr<FAmethystMenuItem> FriendsRoot = FAmethystMenuItem::CreateRoot();

	//Populate the friends list
	FriendsItem = MenuHelper::AddMenuItem(FriendsRoot, LOCTEXT("Friends", "FRIENDS"));
	OnlineSub = IOnlineSubsystem::Get();
	OnlineFriendsPtr = OnlineSub->GetFriendsInterface();
	if ( OnlineFriendsPtr.IsValid() )
	{
		OnlineFriendsPtr->ReadFriendsList(LocalUserNum, EFriendsLists::ToString(EFriendsLists::OnlinePlayers)); //init read of the friends list with the current user
	}

	UserSettings = CastChecked<UAmethystForestUserSettings>(GEngine->GetGameUserSettings());
}

void FAmethystFriends::OnApplySettings()
{
	ApplySettings();
}

void FAmethystFriends::ApplySettings()
{
	UAmethystPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();

		PersistentUser->SaveIfDirty();
	}

	UserSettings->ApplySettings(false);

	OnApplyChanges.ExecuteIfBound();
}

void FAmethystFriends::TellInputAboutKeybindings()
{
	UAmethystPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();
	}
}

UAmethystPersistentUser* FAmethystFriends::GetPersistentUser() const
{
	UAmethystLocalPlayer* const AmethystLocalPlayer = Cast<UAmethystLocalPlayer>(PlayerOwner);
	return AmethystLocalPlayer ? AmethystLocalPlayer->GetPersistentUser() : nullptr;
	//// Main Menu
	//AAmethystPlayerController_Menu* AmethystPCM = Cast<AAmethystPlayerController_Menu>(PCOwner);
	//if(AmethystPCM)
	//{
	//	return AmethystPCM->GetPersistentUser();
	//}

	//// In-game Menu
	//AAmethystPlayerController* AmethystPC = Cast<AAmethystPlayerController>(PCOwner);
	//if(AmethystPC)
	//{
	//	return AmethystPC->GetPersistentUser();
	//}

	//return nullptr;
}

void FAmethystFriends::UpdateFriends(int32 NewOwnerIndex)
{
	if (!OnlineFriendsPtr.IsValid())
	{
		return;
	}

	LocalUserNum = NewOwnerIndex;
	OnlineFriendsPtr->ReadFriendsList(LocalUserNum, EFriendsLists::ToString(EFriendsLists::OnlinePlayers));
	MenuHelper::ClearSubMenu(FriendsItem);

	if (OnlineFriendsPtr->GetFriendsList(LocalUserNum, EFriendsLists::ToString(EFriendsLists::OnlinePlayers), Friends))
	{
		for (int32 Idx = 0; Idx < Friends.Num(); ++Idx)
		{
			const FString FriendUsername = Friends[Idx]->GetDisplayName();
			TSharedPtr<FAmethystMenuItem> FriendItem = MenuHelper::AddMenuItem(FriendsItem, FText::FromString(FriendUsername));
			//FriendItem->OnControllerFacebuttonLeftPressed.BindRaw(this, &FAmethystFriends::InviteSelectedFriendToGame);
			FriendItem->OnControllerDownInputPressed.BindRaw(this, &FAmethystFriends::IncrementFriendsCounter);
			FriendItem->OnControllerUpInputPressed.BindRaw(this, &FAmethystFriends::DecrementFriendsCounter);
			FriendItem->OnControllerFacebuttonDownPressed.BindRaw(this, &FAmethystFriends::ViewSelectedFriendProfile);
		}
		MaxFriendIndex = Friends.Num() - 1;
	}
	MenuHelper::AddMenuItemSP(FriendsItem, LOCTEXT("Close", "CLOSE"), this, &FAmethystFriends::OnApplySettings);
}

void FAmethystFriends::IncrementFriendsCounter()
{
	if (CurrFriendIndex + 1 <= MaxFriendIndex)
	{
		++CurrFriendIndex;
	}
}
void FAmethystFriends::DecrementFriendsCounter()
{
	if (CurrFriendIndex - 1 >= MinFriendIndex)
	{
		--CurrFriendIndex;
	}
}
void FAmethystFriends::ViewSelectedFriendProfile()
{
	auto Identity = Online::GetIdentityInterface();
	if (Identity.IsValid() && Friends.IsValidIndex(CurrFriendIndex))
	{
		TSharedPtr<const FUniqueNetId> Requestor = Identity->GetUniquePlayerId(LocalUserNum);
		TSharedPtr<const FUniqueNetId> Requestee = Friends[CurrFriendIndex]->GetUserId();
		auto ExternalUI = Online::GetExternalUIInterface();
		if (ExternalUI.IsValid() && Requestor.IsValid() && Requestee.IsValid())
		{
			ExternalUI->ShowProfileUI(*Requestor, *Requestee, IOnlineExternalUI::FOnProfileUIClosedDelegate());
		}
	}
}
void FAmethystFriends::InviteSelectedFriendToGame()
{
	if (OnlineFriendsPtr.IsValid() && Friends.IsValidIndex(CurrFriendIndex))
	{
		OnlineFriendsPtr->SendInvite(LocalUserNum, *Friends[CurrFriendIndex]->GetUserId(), EFriendsLists::ToString(EFriendsLists::Default));
	}
}


#undef LOCTEXT_NAMESPACE
