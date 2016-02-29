// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystStyle.h"
#include "SAmethystSplitScreenLobbyWidget.h"
#include "AmethystMenuItemWidgetStyle.h"
#include "AmethystMenuWidgetStyle.h"
#include "SAmethystConfirmationDialog.h"
#include "AmethystForestViewportClient.h"

#define LOCTEXT_NAMESPACE "AmethystForest.SplitScreenLobby"

int32 GAmethystSplitScreenMax = 2;
static FAutoConsoleVariableRef CVarAmethystSplitScreenMax(
	TEXT("r.AmethystSplitScreenMax"),
	GAmethystSplitScreenMax,
	TEXT("Maximum number of split screen players.\n")
	TEXT("This will set the number of slots available in the split screen lobby.\n")
	TEXT("Default is 2."),
	ECVF_Default
	);

void SAmethystSplitScreenLobby::Construct( const FArguments& InArgs )
{
#if PLATFORM_XBOXONE
	PressToPlayText = LOCTEXT("PressToPlay", "Press A to Play");
	PressToFindText = LOCTEXT("PressToFind", "Press A to Find Match");
	PressToStartMatchText = LOCTEXT("PressToStart", "Press A To Start Match");	
#else
	PressToPlayText = LOCTEXT("PressToPlay", "Press cross button to Play");
	PressToFindText = LOCTEXT("PressToFind", "Press cross button to Find Match");
	PressToStartMatchText = LOCTEXT("PressToStart", "Press cross button To Start Match");
#endif	

	PlayerOwner = InArgs._PlayerOwner;

	bIsJoining = false;
	bIsOnline = true;

	const FAmethystMenuStyle* ItemStyle = &FAmethystStyle::Get().GetWidgetStyle<FAmethystMenuStyle>("DefaultAmethystMenuStyle");
	const FSlateBrush* SlotBrush = &ItemStyle->LeftBackgroundBrush;

	const FButtonStyle* ButtonStyle = &FAmethystStyle::Get().GetWidgetStyle<FButtonStyle>("DefaultAmethystButtonStyle");
	FLinearColor MenuTitleTextColor =  FLinearColor(FColor(155,164,182));
	FLinearColor PressXToStartColor = FLinearColor::Green;
	
	MasterUserBack = InArgs._OnCancelClicked;
	MasterUserPlay = InArgs._OnPlayClicked;	

	const float PaddingBetweenSlots = 10.0f;
	const float SlotWidth = 565.0f;
	const float SlotHeight = 300.0f;

	const FText Msg = LOCTEXT("You must be signed in to play online", "You must be signed in to play online");
	const FText OKButtonString = NSLOCTEXT("DialogButtons", "OKAY", "OK");

	ChildSlot
	[	
		//main container.  Just start us out centered on the whole screen.
		SNew(SOverlay)
		+SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			//at maximum we need a 2x2 grid.  So we need two vertical slots, which will each contain 2 horizontal slots.
			SNew( SVerticalBox )
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(PaddingBetweenSlots) //put some space in between the slots
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew( SHorizontalBox)
				+SHorizontalBox::Slot()				
				.Padding(PaddingBetweenSlots) //put some space in between the slots				
				[
					SAssignNew(UserSlots[0], SBox)
					.HeightOverride(SlotHeight)
					.WidthOverride(SlotWidth)
					[
						SNew(SBorder)
						.Padding(0.0f)						
						.BorderImage(SlotBrush)
						.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()												
							.Padding(0.0f)
							.VAlign(VAlign_Bottom)
							.HAlign(HAlign_Center)
							[
								//first slot needs to have some text to say 'press X to start match'. Only master user can start the match.
								SAssignNew(UserTextWidgets[0], STextBlock)
								.TextStyle(FAmethystStyle::Get(), "AmethystForest.MenuHeaderTextStyle")
								.ColorAndOpacity(MenuTitleTextColor)
								.Text(PressToPlayText)														
							]
							+SVerticalBox::Slot()							
							.Padding(5.0f)
							.VAlign(VAlign_Bottom)
							.HAlign(HAlign_Center)					
							[							
								SNew(STextBlock)
								.TextStyle(FAmethystStyle::Get(), "AmethystForest.SplitScreenLobby.StartMatchTextStyle")
								.Text(this, &SAmethystSplitScreenLobby::GetPlayFindText)
							]
						]					
					]
				]
				+SHorizontalBox::Slot()					
				.Padding(PaddingBetweenSlots)				
				[
					SAssignNew(UserSlots[1], SBox)
					.HeightOverride(SlotHeight)
					.WidthOverride(SlotWidth)
					[
						SNew(SBorder)
						.Padding(0.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.BorderImage(SlotBrush)
						.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
						[
							SAssignNew(UserTextWidgets[1], STextBlock)
							.TextStyle(FAmethystStyle::Get(), "AmethystForest.MenuHeaderTextStyle")
							.ColorAndOpacity(MenuTitleTextColor)
							.Text(PressToPlayText)
						]
					]
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(PaddingBetweenSlots)
			[
				SNew( SHorizontalBox)
				+SHorizontalBox::Slot()							
				.Padding(PaddingBetweenSlots)				
				[
					SAssignNew(UserSlots[2], SBox)
					.HeightOverride(SlotHeight)
					.WidthOverride(SlotWidth)
					[
						SNew(SBorder)
						.Padding(0.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.BorderImage(SlotBrush)
						.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
						[
							SAssignNew(UserTextWidgets[2], STextBlock)
							.TextStyle(FAmethystStyle::Get(), "AmethystForest.MenuHeaderTextStyle")
							.ColorAndOpacity(MenuTitleTextColor)
							.Text(PressToPlayText)
						]
					]
				]

				+SHorizontalBox::Slot()				
				.Padding(PaddingBetweenSlots)				
				[
					SAssignNew(UserSlots[3], SBox)
					.HeightOverride(SlotHeight)
					.WidthOverride(SlotWidth)
					[
						SNew(SBorder)
						.Padding(0.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.BorderImage(SlotBrush)
						.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
						[
							SAssignNew(UserTextWidgets[3], STextBlock)
							.TextStyle(FAmethystStyle::Get(), "AmethystForest.MenuHeaderTextStyle")
							.ColorAndOpacity(MenuTitleTextColor)
							.Text(PressToPlayText)
						]
					]
				]	
			]			
		]
	];
	
	Clear();
}

void SAmethystSplitScreenLobby::Clear()
{	
	if (GetGameInstance() == nullptr)
	{
		return;
	}

	// Remove existing splitscreen players
	GetGameInstance()->RemoveSplitScreenPlayers();

	// Sync the list with the actively tracked local users
	UpdateSlots();
}

void SAmethystSplitScreenLobby::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	// Parent tick
	SCompoundWidget::Tick( AllottedGeometry, InCurrentTime, InDeltaTime);

	// Make sure the slots match the local user list
	UpdateSlots();
}

void SAmethystSplitScreenLobby::UpdateSlots()
{
	if (GetGameInstance() == nullptr)
	{
		return;
	}

	// Show active players
	for ( int i = 0; i < GetNumSupportedSlots(); ++i )
	{
		ULocalPlayer * LocalPlayer = ( i < GetGameInstance()->GetNumLocalPlayers() ) ? GetGameInstance()->GetLocalPlayerByIndex( i ) : NULL;

		if ( LocalPlayer != NULL )
		{
			UserTextWidgets[i]->SetText( LocalPlayer->GetNickname() );
		}
		else
		{ 
			UserTextWidgets[i]->SetText( PressToPlayText );
		}

		UserSlots[i]->SetVisibility( EVisibility::Visible );
	}
	
	// Hide non supported slots
	for ( int i = GetNumSupportedSlots(); i < MAX_POSSIBLE_SLOTS; ++i )
	{
		UserSlots[i]->SetVisibility( EVisibility::Collapsed );
	}
}

void SAmethystSplitScreenLobby::ConditionallyReadyPlayer( const int ControllerId, const bool bCanShowUI )
{
	if (GetGameInstance() == nullptr)
	{
		return;
	}
	
	if ( GetGameInstance()->GetNumLocalPlayers() >= GetNumSupportedSlots() )
	{
		return;		// Can't fit any more players at this point
	}

	// If we already have this controller id registered, ignore this request
	if ( GetGameInstance()->FindLocalPlayerFromControllerId( ControllerId ) != NULL )
	{
		return;
	}

	TSharedPtr< const FUniqueNetId > UniqueNetId = GetGameInstance()->GetUniqueNetIdFromControllerId( ControllerId );

	const bool bIsPlayerOnline		= ( UniqueNetId.IsValid() && IsUniqueIdOnline( *UniqueNetId ) );
	const bool bFoundExistingPlayer = GetGameInstance()->FindLocalPlayerFromUniqueNetId( UniqueNetId ) != nullptr;

	// If this is an online game, reject and show sign-in if:
	//	1. We have already registered this FUniqueNetId
	//	2. This player is not currently signed in and online
	if ( bIsOnline && ( bFoundExistingPlayer || !bIsPlayerOnline ) )
	{
		const auto ExternalUI = Online::GetExternalUIInterface();

		if ( bCanShowUI && ExternalUI.IsValid() )
		{
			ExternalUI->ShowLoginUI( ControllerId, false, IOnlineExternalUI::FOnLoginUIClosedDelegate::CreateSP( this, &SAmethystSplitScreenLobby::HandleLoginUIClosedAndReady ) );
		}

		return;
	}

	if (bIsPlayerOnline)
	{
		const auto OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			const auto Identity = OnlineSub->GetIdentityInterface();
			if(Identity.IsValid())
			{
				PendingControllerId = ControllerId;
				Identity->GetUserPrivilege(
					*UniqueNetId,
					bIsOnline ? EUserPrivileges::CanPlayOnline : EUserPrivileges::CanPlay,
					IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateSP(this, &SAmethystSplitScreenLobby::OnUserCanPlay));
			}
		}
	}
	else
	{
		ReadyPlayer(ControllerId);
	}
}

void SAmethystSplitScreenLobby::ReadyPlayer( const int ControllerId )
{
	FString Error;
	ULocalPlayer * LocalPlayer = GetGameInstance()->CreateLocalPlayer(ControllerId, Error, false);

	if (LocalPlayer != NULL)
	{
		// We have UserId, but we want to make sure we're using the same shared pointer from the game instance so all the reference counting works out
		TSharedPtr< const FUniqueNetId > UniqueNetId = GetGameInstance()->GetUniqueNetIdFromControllerId(ControllerId);
		LocalPlayer->SetCachedUniqueNetId(UniqueNetId);
	}
}

void SAmethystSplitScreenLobby::UnreadyPlayer( const int ControllerId )
{
	if (GetGameInstance() == nullptr)
	{
		return;
	}

	ULocalPlayer * LocalPlayer = GEngine->GetLocalPlayerFromControllerId( GetGameInstance()->GetGameViewportClient(), ControllerId );

	if ( LocalPlayer == NULL )
	{
		UE_LOG( LogOnline, Warning, TEXT( "SAmethystSplitScreenLobby::UnreadyPlayer: ControllerId not found: %i" ), ControllerId );
		return;
	}

	GetGameInstance()->RemoveExistingLocalPlayer( LocalPlayer );
}

FReply SAmethystSplitScreenLobby::OnOkOrCancel()
{
	UAmethystForestViewportClient* AmethystViewport = Cast<UAmethystForestViewportClient>(GetGameInstance()->GetGameViewportClient());

	if (AmethystViewport != NULL)
	{
		AmethystViewport->HideDialog();
	}

	return FReply::Handled();
}

bool SAmethystSplitScreenLobby::ConfirmSponsorsSatisfied() const
{
	if (GetGameInstance() == nullptr)
	{
		return false;
	}

	const auto OnlineSub = IOnlineSubsystem::Get();

	if(OnlineSub)
	{
		const auto IdentityInterface = OnlineSub->GetIdentityInterface();
		if(IdentityInterface.IsValid())
		{
			const int32 NumActiveSlots = FMath::Min( GetNumSupportedSlots(), GetGameInstance()->GetNumLocalPlayers() );

			for ( int i = 0; i < NumActiveSlots; ++i )
			{
				ULocalPlayer * LocalPlayer = GetGameInstance()->GetLocalPlayerByIndex( i );

				TSharedPtr<const FUniqueNetId> Sponsor = IdentityInterface->GetSponsorUniquePlayerId( LocalPlayer->GetControllerId() );

				// If this user has a sponsor (is a guest), make sure our sponsor is playing with us
				if(Sponsor.IsValid())
				{
					return ( GetGameInstance()->FindLocalPlayerFromUniqueNetId( Sponsor ) != NULL );
				}
			}
		}
	}

	return true;
}

void SAmethystSplitScreenLobby::OnUserCanPlay(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	if (PrivilegeResults != (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{
		// Xbox shows its own system dialog currently
#if PLATFORM_PS4
		const auto OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			const auto Identity = OnlineSub->GetIdentityInterface();
			if (Identity.IsValid())
			{
				FString Nickname = Identity->GetPlayerNickname(UserId);

				// Show warning that the user cannot play due to age restrictions
				UAmethystForestViewportClient * AmethystViewport = Cast<UAmethystForestViewportClient>(GetGameInstance()->GetGameViewportClient());

				if (AmethystViewport != NULL)
				{
					AmethystViewport->ShowDialog(
						PlayerOwner.Get(),
						EAmethystDialogType::Generic,
						FText::Format(NSLOCTEXT("ProfileMessages", "AgeRestrictionFmt", "{0} cannot play due to age restrictions!"), FText::FromString(Nickname)),
						NSLOCTEXT("DialogButtons", "OKAY", "OK"),
						FText::GetEmpty(),
						FOnClicked::CreateRaw(this, &SAmethystSplitScreenLobby::OnOkOrCancel),
						FOnClicked::CreateRaw(this, &SAmethystSplitScreenLobby::OnOkOrCancel)
						);
				}
			}
		}
#endif
	}
	else
	{
		ReadyPlayer(PendingControllerId);
	}
}

int32 SAmethystSplitScreenLobby::GetNumSupportedSlots() const
{
	return FMath::Clamp( GAmethystSplitScreenMax, 1, MAX_POSSIBLE_SLOTS );
}

bool SAmethystSplitScreenLobby::IsUniqueIdOnline( const FUniqueNetId& UniqueId ) const
{
	auto Identity = Online::GetIdentityInterface();

	if ( !Identity.IsValid() )
	{
		return false;
	}

	return Identity->GetLoginStatus( UniqueId ) == ELoginStatus::LoggedIn;
}

FReply SAmethystSplitScreenLobby::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (GetGameInstance() == nullptr)
	{
		return FReply::Unhandled();
	}

	const FKey Key = InKeyEvent.GetKey();
	const int32 UserIndex = InKeyEvent.GetUserIndex();

	ULocalPlayer * ExistingPlayer = GEngine->GetLocalPlayerFromControllerId(GetGameInstance()->GetGameViewportClient(), UserIndex);

	if ((Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom) && !InKeyEvent.IsRepeat())
	{
		// See if we are already tracking this user
		if ( ExistingPlayer != NULL )
		{
			// See if this is the initial user
			if ( ExistingPlayer == GetGameInstance()->GetFirstGamePlayer() )
			{
				// If this is the initial user, start the game
				if ( !bIsOnline || ConfirmSponsorsSatisfied() )
				{
					return MasterUserPlay.Execute();
				}
				else
				{
					// Show warning that the guest needs the sponsor
					UAmethystForestViewportClient * AmethystViewport = Cast<UAmethystForestViewportClient>(GetGameInstance()->GetGameViewportClient());

					if ( AmethystViewport != NULL )
					{
						AmethystViewport->ShowDialog(
							PlayerOwner.Get(),
							EAmethystDialogType::Generic,
							NSLOCTEXT("ProfileMessages", "GuestAccountNeedsSponsor", "A guest account cannot play without its sponsor!"),
							NSLOCTEXT("DialogButtons", "OKAY", "OK"),
							FText::GetEmpty(),
							FOnClicked::CreateRaw(this, &SAmethystSplitScreenLobby::OnOkOrCancel),
							FOnClicked::CreateRaw(this, &SAmethystSplitScreenLobby::OnOkOrCancel)
						);
					}

					return FReply::Handled();
				}
			}
	
			return FReply::Handled();
		}

		ConditionallyReadyPlayer(UserIndex, true);

		return FReply::Handled();
	}
	else if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Global_Back)
	{
		if ( ExistingPlayer != NULL && ExistingPlayer == GetGameInstance()->GetFirstGamePlayer() )
		{
			return MasterUserBack.Execute();
		}
		else
		{
			UnreadyPlayer(UserIndex);
		}
	}

	return FReply::Unhandled();
}

FReply SAmethystSplitScreenLobby::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	return FReply::Handled().ReleaseMouseCapture().SetUserFocus(SharedThis( this ), EFocusCause::SetDirectly, true);
}

void SAmethystSplitScreenLobby::OnFocusLost( const FFocusEvent& InFocusEvent )
{
}

void SAmethystSplitScreenLobby::HandleLoginUIClosedAndReady( TSharedPtr<const FUniqueNetId> UniqueId, const int UserIndex )
{
	// If a player signed in, UniqueId will be valid, and we can place him in the open slot.
	if ( UniqueId.IsValid() )
	{
		if ( !bIsOnline || IsUniqueIdOnline( *UniqueId ) )
		{
			ConditionallyReadyPlayer( UserIndex, false );
		}
	}
}

UAmethystForestInstance * SAmethystSplitScreenLobby::GetGameInstance() const
{
	if ( !PlayerOwner.IsValid() )
	{
		return NULL;
	}

	check( PlayerOwner->GetGameInstance() == nullptr || CastChecked< UAmethystForestInstance >( PlayerOwner->GetGameInstance() ) != nullptr );

	return Cast< UAmethystForestInstance >( PlayerOwner->GetGameInstance() );
}

FText SAmethystSplitScreenLobby::GetPlayFindText() const
{
	return bIsJoining ? PressToFindText : PressToStartMatchText;
}

#undef LOCTEXT_NAMESPACE
