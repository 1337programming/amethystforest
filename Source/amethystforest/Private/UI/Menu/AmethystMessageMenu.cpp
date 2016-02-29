// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystStyle.h"
#include "SAmethystConfirmationDialog.h"
#include "AmethystMessageMenu.h"
#include "AmethystForestViewportClient.h"
#include "AmethystForestInstance.h"

#define LOCTEXT_NAMESPACE "AmethystForest.HUD.Menu"

void FAmethystMessageMenu::Construct(TWeakObjectPtr<UAmethystForestInstance> InGameInstance, TWeakObjectPtr<ULocalPlayer> InPlayerOwner, const FText& Message, const FText& OKButtonText, const FText& CancelButtonText, const FName& InPendingNextState)
{
	GameInstance			= InGameInstance;
	PlayerOwner				= InPlayerOwner;
	PendingNextState		= InPendingNextState;

	if ( ensure( GameInstance.IsValid() ) )
	{
		UAmethystForestViewportClient* AmethystViewport = Cast<UAmethystForestViewportClient>( GameInstance->GetGameViewportClient() );

		if ( AmethystViewport )
		{
			// Hide the previous dialog
			AmethystViewport->HideDialog();

			// Show the new one
			AmethystViewport->ShowDialog(
				PlayerOwner,
				EAmethystDialogType::Generic,
				Message, 
				OKButtonText, 
				CancelButtonText, 
				FOnClicked::CreateRaw(this, &FAmethystMessageMenu::OnClickedOK),
				FOnClicked::CreateRaw(this, &FAmethystMessageMenu::OnClickedCancel)
			);
		}
	}
}

void FAmethystMessageMenu::RemoveFromGameViewport()
{
	if ( ensure( GameInstance.IsValid() ) )
	{
		UAmethystForestViewportClient * AmethystViewport = Cast<UAmethystForestViewportClient>( GameInstance->GetGameViewportClient() );

		if ( AmethystViewport )
		{
			// Hide the previous dialog
			AmethystViewport->HideDialog();
		}
	}
}

void FAmethystMessageMenu::HideDialogAndGotoNextState()
{
	RemoveFromGameViewport();

	if ( ensure( GameInstance.IsValid() ) )
	{
		GameInstance->GotoState( PendingNextState );
	}
};

FReply FAmethystMessageMenu::OnClickedOK()
{
	OKButtonDelegate.ExecuteIfBound();
	HideDialogAndGotoNextState();
	return FReply::Handled();
}

FReply FAmethystMessageMenu::OnClickedCancel()
{
	CancelButtonDelegate.ExecuteIfBound();
	HideDialogAndGotoNextState();
	return FReply::Handled();
}

void FAmethystMessageMenu::SetOKClickedDelegate(FMessageMenuButtonClicked InButtonDelegate)
{
	OKButtonDelegate = InButtonDelegate;
}

void FAmethystMessageMenu::SetCancelClickedDelegate(FMessageMenuButtonClicked InButtonDelegate)
{
	CancelButtonDelegate = InButtonDelegate;
}


#undef LOCTEXT_NAMESPACE
