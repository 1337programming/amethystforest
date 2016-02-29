// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystWelcomeMenu.h"
#include "AmethystStyle.h"
#include "SAmethystConfirmationDialog.h"
#include "AmethystForestViewportClient.h"
#include "AmethystForestInstance.h"


#define LOCTEXT_NAMESPACE "AmethystForest.HUD.Menu"

class SAmethystWelcomeMenuWidget : public SCompoundWidget
{
	/** The menu that owns this widget. */
	FAmethystWelcomeMenu* MenuOwner;

	/** Animate the text so that the screen isn't static, for console cert requirements. */
	FCurveSequence TextAnimation;

	/** The actual curve that animates the text. */
	FCurveHandle TextColorCurve;

	SLATE_BEGIN_ARGS( SAmethystWelcomeMenuWidget )
	{}

	SLATE_ARGUMENT(FAmethystWelcomeMenu*, MenuOwner)

	SLATE_END_ARGS()

	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

	/**
	 * Gets the text color based on the current state of the animation.
	 *
	 * @return The text color based on the current state of the animation.
	 */
	FSlateColor GetTextColor() const
	{
		return FSlateColor(FMath::Lerp(FLinearColor(0.0f,0.0f,0.0f,1.0f), FLinearColor(0.5f,0.5f,0.5f,1.0f), TextColorCurve.GetLerp()));
	}

	void Construct( const FArguments& InArgs )
	{
		MenuOwner = InArgs._MenuOwner;
		
		TextAnimation = FCurveSequence();
		const float AnimDuration = 1.5f;
		TextColorCurve = TextAnimation.AddCurve(0, AnimDuration, ECurveEaseFunction::QuadInOut);

		ChildSlot
		[
			SNew(SBorder)
			.Padding(30.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[ 
				SNew( STextBlock )
#if PLATFORM_PS4
				.Text( LOCTEXT("PressStartPS4", "PRESS CROSS BUTTON TO PLAY" ) )
#else
				.Text( LOCTEXT("PressStartXboxOne", "PRESS A TO PLAY" ) )
#endif
				.ColorAndOpacity(this, &SAmethystWelcomeMenuWidget::GetTextColor)
				.TextStyle( FAmethystStyle::Get(), "AmethystForest.WelcomeScreen.WelcomeTextStyle" )
			]
		];
	}

	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override
	{
		if(!TextAnimation.IsPlaying())
		{
			if(TextAnimation.IsAtEnd())
			{
				TextAnimation.PlayReverse(this->AsShared());
			}
			else
			{
				TextAnimation.Play(this->AsShared());
			}
		}
	}

	virtual FReply OnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override
	{
		return FReply::Handled();
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		const FKey Key = InKeyEvent.GetKey();
		if (Key == EKeys::Enter)
		{
			MenuOwner->HandleLoginUIClosed(TSharedPtr<const FUniqueNetId>(), 0);
		}
		else if (!MenuOwner->GetControlsLocked() && Key == EKeys::Gamepad_FaceButton_Bottom)
		{
			bool bSkipToMainMenu = true;

			{
				const auto OnlineSub = IOnlineSubsystem::Get();
				if (OnlineSub)
				{
					const auto IdentityInterface = OnlineSub->GetIdentityInterface();
					if (IdentityInterface.IsValid())
					{
						TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
						const bool bIsLicensed = GenericApplication->ApplicationLicenseValid();

						const auto LoginStatus = IdentityInterface->GetLoginStatus(InKeyEvent.GetUserIndex());
						if (LoginStatus == ELoginStatus::NotLoggedIn || !bIsLicensed)
						{
							// Show the account picker.
							const auto ExternalUI = OnlineSub->GetExternalUIInterface();
							if (ExternalUI.IsValid())
							{
								ExternalUI->ShowLoginUI(InKeyEvent.GetUserIndex(), false, IOnlineExternalUI::FOnLoginUIClosedDelegate::CreateSP(MenuOwner, &FAmethystWelcomeMenu::HandleLoginUIClosed));
								bSkipToMainMenu = false;
							}
						}
					}
				}
			}

			if (bSkipToMainMenu)
			{
				const auto OnlineSub = IOnlineSubsystem::Get();
				if (OnlineSub)
				{
					const auto IdentityInterface = OnlineSub->GetIdentityInterface();
					if (IdentityInterface.IsValid())
					{
						TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(InKeyEvent.GetUserIndex());
						// If we couldn't show the external login UI for any reason, or if the user is
						// already logged in, just advance to the main menu immediately.
						MenuOwner->HandleLoginUIClosed(UserId, InKeyEvent.GetUserIndex());
					}
				}
			}

			return FReply::Handled();
		}

		return FReply::Unhandled();
	}

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override
	{
		return FReply::Handled().ReleaseMouseCapture().SetUserFocus(SharedThis( this ), EFocusCause::SetDirectly, true);
	}
};

void FAmethystWelcomeMenu::Construct( TWeakObjectPtr< UAmethystForestInstance > InGameInstance )
{
	bControlsLocked = false;
	GameInstance = InGameInstance;
	PendingControllerIndex = -1;

	MenuWidget = SNew( SAmethystWelcomeMenuWidget )
		.MenuOwner(this);	
}

void FAmethystWelcomeMenu::AddToGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(MenuWidget.ToSharedRef());
		FSlateApplication::Get().SetKeyboardFocus(MenuWidget);
	}
}

void FAmethystWelcomeMenu::RemoveFromGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidget.ToSharedRef());
	}
}

void FAmethystWelcomeMenu::HandleLoginUIClosed(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex)
{
	if ( !ensure( GameInstance.IsValid() ) )
	{
		return;
	}

	UAmethystForestViewportClient* AmethystViewport = Cast<UAmethystForestViewportClient>( GameInstance->GetGameViewportClient() );

	TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
	const bool bIsLicensed = GenericApplication->ApplicationLicenseValid();

	// If they don't currently have a license, let them know, but don't let them proceed
	if (!bIsLicensed && AmethystViewport != NULL)
	{
		const FText StopReason	= NSLOCTEXT( "ProfileMessages", "NeedLicense", "The signed in users do not have a license for this game. Please purchase AmethystForest from the Xbox Marketplace or sign in a user with a valid license." );
		const FText OKButton	= NSLOCTEXT( "DialogButtons", "OKAY", "OK" );

		AmethystViewport->ShowDialog(
			nullptr,
			EAmethystDialogType::Generic,
			StopReason,
			OKButton,
			FText::GetEmpty(),
			FOnClicked::CreateRaw(this, &FAmethystWelcomeMenu::OnConfirmGeneric),
			FOnClicked::CreateRaw(this, &FAmethystWelcomeMenu::OnConfirmGeneric)
			);
		return;
	}

	PendingControllerIndex = ControllerIndex;

	if(UniqueId.IsValid())
	{
		// Next step, check privileges
		const auto OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			const auto IdentityInterface = OnlineSub->GetIdentityInterface();
			if (IdentityInterface.IsValid())
			{
				IdentityInterface->GetUserPrivilege(*UniqueId, EUserPrivileges::CanPlay, IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateSP(this, &FAmethystWelcomeMenu::OnUserCanPlay));
			}
		}
	}
	else
	{
		// Show a warning that your progress won't be saved if you continue without logging in. 
		if (AmethystViewport != NULL)
		{
			AmethystViewport->ShowDialog(
				nullptr,
				EAmethystDialogType::Generic,
				NSLOCTEXT("ProfileMessages", "ProgressWillNotBeSaved", "If you continue without signing in, your progress will not be saved."),
				NSLOCTEXT("DialogButtons", "AContinue", "A - Continue"),
				NSLOCTEXT("DialogButtons", "BBack", "B - Back"),
				FOnClicked::CreateRaw(this, &FAmethystWelcomeMenu::OnContinueWithoutSavingConfirm),
				FOnClicked::CreateRaw(this, &FAmethystWelcomeMenu::OnConfirmGeneric)
			);
		}
	}
}

void FAmethystWelcomeMenu::SetControllerAndAdvanceToMainMenu(const int ControllerIndex)
{
	if ( !ensure( GameInstance.IsValid() ) )
	{
		return;
	}

	ULocalPlayer * NewPlayerOwner = GameInstance->GetFirstGamePlayer();

	if ( NewPlayerOwner != nullptr && ControllerIndex != -1 )
	{
		NewPlayerOwner->SetControllerId(ControllerIndex);
		NewPlayerOwner->SetCachedUniqueNetId(NewPlayerOwner->GetUniqueNetIdFromCachedControllerId());

		// tell gameinstance to transition to main menu
		GameInstance->GotoState(AmethystForestInstanceState::MainMenu);
	}	
}

FReply FAmethystWelcomeMenu::OnContinueWithoutSavingConfirm()
{
	if ( !ensure( GameInstance.IsValid() ) )
	{
		return FReply::Handled();
	}

	UAmethystForestViewportClient * AmethystViewport = Cast<UAmethystForestViewportClient>( GameInstance->GetGameViewportClient() );

	if (AmethystViewport != NULL)
	{
		AmethystViewport->HideDialog();
	}

	SetControllerAndAdvanceToMainMenu(PendingControllerIndex);
	return FReply::Handled();
}

FReply FAmethystWelcomeMenu::OnConfirmGeneric()
{
	if ( !ensure( GameInstance.IsValid() ) )
	{
		return FReply::Handled();
	}

	UAmethystForestViewportClient * AmethystViewport = Cast<UAmethystForestViewportClient>( GameInstance->GetGameViewportClient() );

	if (AmethystViewport != NULL)
	{
		AmethystViewport->HideDialog();
	}

	return FReply::Handled();
}

void FAmethystWelcomeMenu::OnUserCanPlay(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	if (PrivilegeResults == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{
		SetControllerAndAdvanceToMainMenu(PendingControllerIndex);
	}
	else
	{
		UAmethystForestViewportClient * AmethystViewport = Cast<UAmethystForestViewportClient>( GameInstance->GetGameViewportClient() );

		if ( AmethystViewport != NULL )
		{
			const FText ReturnReason = NSLOCTEXT("PrivilegeFailures", "CannotPlayAgeRestriction", "You cannot play this game due to age restrictions.");
			const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

			AmethystViewport->ShowDialog(
				nullptr,
				EAmethystDialogType::Generic,
				ReturnReason,
				OKButton,
				FText::GetEmpty(),
				FOnClicked::CreateRaw(this, &FAmethystWelcomeMenu::OnConfirmGeneric),
				FOnClicked::CreateRaw(this, &FAmethystWelcomeMenu::OnConfirmGeneric)
			);
		}
	}
}

#undef LOCTEXT_NAMESPACE
