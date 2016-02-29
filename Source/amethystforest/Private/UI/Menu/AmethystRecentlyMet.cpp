// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystRecentlyMet.h"
#include "AmethystTypes.h"
#include "AmethystStyle.h"
#include "AmethystOptionsWidgetStyle.h"
#include "AmethystForestUserSettings.h"
#include "AmethystPersistentUser.h"
#include "Player/AmethystLocalPlayer.h"

#define LOCTEXT_NAMESPACE "AmethystForest.HUD.Menu"

void FAmethystRecentlyMet::Construct(ULocalPlayer* _PlayerOwner, int32 LocalUserNum_)
{
	RecentlyMetStyle = &FAmethystStyle::Get().GetWidgetStyle<FAmethystOptionsStyle>("DefaultAmethystOptionsStyle");

	PlayerOwner = _PlayerOwner;
	LocalUserNum = LocalUserNum_;
	CurrRecentlyMetIndex = 0;
	MinRecentlyMetIndex = 0;
	MaxRecentlyMetIndex = 0; //initialized after the recently met list is (read in/set)

	/** Recently Met menu items */
	RecentlyMetRoot = FAmethystMenuItem::CreateRoot();
	RecentlyMetItem = MenuHelper::AddMenuItem(RecentlyMetRoot, LOCTEXT("Recently Met", "RECENTLY MET"));

	/** Init online items */
	OnlineSub = IOnlineSubsystem::Get();

	UserSettings = CastChecked<UAmethystForestUserSettings>(GEngine->GetGameUserSettings());
}

void FAmethystRecentlyMet::OnApplySettings()
{
	ApplySettings();
}

void FAmethystRecentlyMet::ApplySettings()
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

void FAmethystRecentlyMet::TellInputAboutKeybindings()
{
	UAmethystPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();
	}
}

UAmethystPersistentUser* FAmethystRecentlyMet::GetPersistentUser() const
{
	UAmethystLocalPlayer* const SLP = Cast<UAmethystLocalPlayer>(PlayerOwner);
	return SLP ? SLP->GetPersistentUser() : nullptr;
}

void FAmethystRecentlyMet::UpdateRecentlyMet(int32 NewOwnerIndex)
{
	LocalUserNum = NewOwnerIndex;
	
	auto Identity = Online::GetIdentityInterface();
	if (Identity.IsValid())
	{
		LocalUsername = Identity->GetPlayerNickname(LocalUserNum);
	}

	MenuHelper::ClearSubMenu(RecentlyMetItem);
	MaxRecentlyMetIndex = 0;

	AAmethystForestState* const MyGameState = Cast<AAmethystForestState>(PlayerOwner->GetWorld()->GameState);
	if (MyGameState != nullptr)
	{
		PlayerArray = MyGameState->PlayerArray;

		for (int32 i = 0; i < PlayerArray.Num(); ++i)
		{
			const APlayerState* PlayerState = PlayerArray[i];
			FString Username = PlayerState->GetHumanReadableName();
			if (Username != LocalUsername && PlayerState->bIsABot == false)
			{
				TSharedPtr<FAmethystMenuItem> UserItem = MenuHelper::AddMenuItem(RecentlyMetItem, FText::FromString(Username));
				UserItem->OnControllerDownInputPressed.BindRaw(this, &FAmethystRecentlyMet::IncrementRecentlyMetCounter);
				UserItem->OnControllerUpInputPressed.BindRaw(this, &FAmethystRecentlyMet::DecrementRecentlyMetCounter);
				UserItem->OnControllerFacebuttonDownPressed.BindRaw(this, &FAmethystRecentlyMet::ViewSelectedUsersProfile);
			}
			else
			{
				PlayerArray.RemoveAt(i);
				--i; //we just deleted an item, so we need to go make sure i doesn't increment again, otherwise it would skip the player that was supposed to be looked at next
			}
		}

		MaxRecentlyMetIndex = PlayerArray.Num() - 1;
	}

	MenuHelper::AddMenuItemSP(RecentlyMetItem, LOCTEXT("Close", "CLOSE"), this, &FAmethystRecentlyMet::OnApplySettings);
}

void FAmethystRecentlyMet::IncrementRecentlyMetCounter()
{
	if (CurrRecentlyMetIndex + 1 <= MaxRecentlyMetIndex)
	{
		++CurrRecentlyMetIndex;
	}
}
void FAmethystRecentlyMet::DecrementRecentlyMetCounter()
{
	if (CurrRecentlyMetIndex - 1 >= MinRecentlyMetIndex)
	{
		--CurrRecentlyMetIndex;
	}
}
void FAmethystRecentlyMet::ViewSelectedUsersProfile()
{
	auto Identity = Online::GetIdentityInterface();
	if (Identity.IsValid() && PlayerArray.IsValidIndex(CurrRecentlyMetIndex))
	{
		const APlayerState* PlayerState = PlayerArray[CurrRecentlyMetIndex];
		TSharedPtr<const FUniqueNetId> Requestor = Identity->GetUniquePlayerId(LocalUserNum);
		TSharedPtr<const FUniqueNetId> Requestee = PlayerState->UniqueId.GetUniqueNetId();
		auto ExternalUI = Online::GetExternalUIInterface();
		if (ExternalUI.IsValid() && Requestor.IsValid() && Requestee.IsValid())
		{
			ExternalUI->ShowProfileUI(*Requestor, *Requestee, IOnlineExternalUI::FOnProfileUIClosedDelegate());
		}
	}
}


#undef LOCTEXT_NAMESPACE
