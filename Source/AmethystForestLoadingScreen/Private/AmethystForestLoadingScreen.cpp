// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForestLoadingScreen.h"
#include "GenericApplication.h"
#include "GenericApplicationMessageHandler.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "MoviePlayer.h"

// This module must be loaded "PreLoadingScreen" in the .uproject file, otherwise it will not hook in time!

struct FAmethystForestLoadingScreenBrush : public FSlateDynamicImageBrush, public FGCObject
{
	FAmethystForestLoadingScreenBrush( const FName InTextureName, const FVector2D& InImageSize )
		: FSlateDynamicImageBrush( InTextureName, InImageSize )
	{
		ResourceObject = LoadObject<UObject>( NULL, *InTextureName.ToString() );
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector)
	{
		if( ResourceObject )
		{
			Collector.AddReferencedObject(ResourceObject);
		}
	}
};

class SAmethystLoadingScreen2 : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAmethystLoadingScreen2) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		static const FName LoadingScreenName(TEXT("/Game/UI/Menu/LoadingScreen.LoadingScreen"));

		//since we are not using game styles here, just load one image
		LoadingScreenBrush = MakeShareable( new FAmethystForestLoadingScreenBrush( LoadingScreenName, FVector2D(1920,1080) ) );

		ChildSlot
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(LoadingScreenBrush.Get())
			]
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SSafeZone)
				.VAlign(VAlign_Bottom)
				.HAlign(HAlign_Right)
				.Padding(10.0f)
				.IsTitleSafe(true)
				[
					SNew(SThrobber)
					.Visibility(this, &SAmethystLoadingScreen2::GetLoadIndicatorVisibility)
				]
			]
		];
	}

private:
	EVisibility GetLoadIndicatorVisibility() const
	{
		return EVisibility::Visible;
	}

	/** loading screen image brush */
	TSharedPtr<FSlateDynamicImageBrush> LoadingScreenBrush;
};

class FAmethystForestLoadingScreenModule : public IAmethystForestLoadingScreenModule
{
public:
	virtual void StartupModule() override
	{		
		// Load for cooker reference
		LoadObject<UObject>(NULL, TEXT("/Game/UI/Menu/LoadingScreen.LoadingScreen") );

		if (IsMoviePlayerEnabled())
		{
			FLoadingScreenAttributes LoadingScreen;
			LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
			LoadingScreen.MoviePaths.Add(TEXT("LoadingScreen"));
			GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
		}
	}
	
	virtual bool IsGameModule() const override
	{
		return true;
	}

	virtual void StartInGameLoadingScreen() override
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreen.WidgetLoadingScreen = SNew(SAmethystLoadingScreen2);

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
};

IMPLEMENT_GAME_MODULE(FAmethystForestLoadingScreenModule, AmethystForestLoadingScreen);