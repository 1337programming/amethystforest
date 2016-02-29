// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "SAmethystDemoHUD.h"
#include "Engine/DemoNetDriver.h"
#include "AmethystStyle.h"
#include "CoreStyle.h"

/** Widget to represent the main replay timeline bar */
class SAmethystReplayTimeline : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAmethystReplayTimeline)
		: _DemoDriver(nullptr)
		, _BackgroundBrush( FCoreStyle::Get().GetDefaultBrush() )
		, _IndicatorBrush( FCoreStyle::Get().GetDefaultBrush() )
		{}
	SLATE_ARGUMENT(TWeakObjectPtr<UDemoNetDriver>, DemoDriver)
	SLATE_ATTRIBUTE( FMargin, BackgroundPadding )
	SLATE_ATTRIBUTE( const FSlateBrush*, BackgroundBrush )
	SLATE_ATTRIBUTE( const FSlateBrush*, IndicatorBrush )
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	/** Jumps the replay to the time on the bar that was clicked */
	FReply OnTimelineClicked(const FGeometry& Geometry, const FPointerEvent& Event);

	/** The demo net driver underlying the current replay */
	TWeakObjectPtr<UDemoNetDriver> DemoDriver;

	/** The FName of the image resource to show */
	TAttribute< const FSlateBrush* > BackgroundBrush;

	/** The FName of the image resource to show */
	TAttribute< const FSlateBrush* > IndicatorBrush;
};

void SAmethystReplayTimeline::Construct(const FArguments& InArgs)
{
	DemoDriver = InArgs._DemoDriver;
	BackgroundBrush = InArgs._BackgroundBrush;
	IndicatorBrush = InArgs._IndicatorBrush;

	ChildSlot
	.Padding(InArgs._BackgroundPadding)
	[
		SNew(SBorder)
		.BorderImage(BackgroundBrush)
		.OnMouseButtonDown(this, &SAmethystReplayTimeline::OnTimelineClicked)
	];
}

FVector2D SAmethystReplayTimeline::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	// Just use the size of the indicator image here
	const FSlateBrush* ImageBrush = IndicatorBrush.Get();
	if (ImageBrush != nullptr)
	{
		return ImageBrush->ImageSize;
	}
	return FVector2D::ZeroVector;
}

int32 SAmethystReplayTimeline::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const int32 ParentLayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// Manually draw the position indicator
	const FSlateBrush* ImageBrush = IndicatorBrush.Get();
	if ((ImageBrush != nullptr) && (ImageBrush->DrawAs != ESlateBrushDrawType::NoDrawType) && DemoDriver.IsValid())
	{
		const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);
		const uint32 DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

		const FLinearColor FinalColorAndOpacity( InWidgetStyle.GetColorAndOpacityTint() * ColorAndOpacity.Get() * ImageBrush->GetTint( InWidgetStyle ) );

		// Adjust clipping rect to replay time
		const float ReplayPercent = DemoDriver->DemoCurrentTime / DemoDriver->DemoTotalTime;

		const FVector2D Center(
			AllottedGeometry.GetLocalSize().X * ReplayPercent,
			AllottedGeometry.GetLocalSize().Y * 0.5f
		);
		
		const FVector2D Offset = Center - ImageBrush->ImageSize * 0.5f;

		const int32 IndicatorLayerId = ParentLayerId + 1;
			
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			IndicatorLayerId,
			AllottedGeometry.ToPaintGeometry(Offset, ImageBrush->ImageSize),
			ImageBrush,
			MyClippingRect,
			DrawEffects,
			FinalColorAndOpacity
		);

		return IndicatorLayerId;
	}

	return ParentLayerId;
}

FReply SAmethystReplayTimeline::OnTimelineClicked(const FGeometry& Geometry, const FPointerEvent& Event)
{
	if (DemoDriver.IsValid())
	{
		const FVector2D LocalPos = Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition());

		const float TimelinePercentage = LocalPos.X / Geometry.GetLocalSize().X;

		DemoDriver->GotoTimeInSeconds( TimelinePercentage * DemoDriver->DemoTotalTime );

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SAmethystDemoHUD::Construct(const FArguments& InArgs)
{	
	PlayerOwner = InArgs._PlayerOwner;
	check(PlayerOwner.IsValid());

	ChildSlot
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot()
		.FillHeight(8.5f)
		.VAlign(VAlign_Bottom)
		[		
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(1.5f)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Margin(3.0f)
				.Text(this, &SAmethystDemoHUD::GetCurrentReplayTime)
			]

			+SHorizontalBox::Slot()
			.FillWidth(7.0f)
			[

				SNew(SOverlay)
				+SOverlay::Slot()
				[
					SNew(SAmethystReplayTimeline)
					.DemoDriver(PlayerOwner->GetWorld()->DemoNetDriver)
					.BackgroundBrush(FAmethystStyle::Get().GetBrush("AmethystForest.ReplayTimelineBorder"))
					.BackgroundPadding(FMargin(0.0f, 3.0))
					.IndicatorBrush(FAmethystStyle::Get().GetBrush("AmethystForest.ReplayTimelineIndicator"))
				]

				+SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Visibility(EVisibility::HitTestInvisible)
					.Margin(3.0f)
					.Text(this, &SAmethystDemoHUD::GetPlaybackSpeed)
				]
			]

			+SHorizontalBox::Slot()
			.FillWidth(1.5f)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Margin(3.0f)
				.Text(this, &SAmethystDemoHUD::GetTotalReplayTime)
			]
		]

		+SVerticalBox::Slot()
		.FillHeight(1.5f)
		.HAlign(HAlign_Center)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(FMargin(6.0))
			.AutoHeight()
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked(this, &SAmethystDemoHUD::IsPauseChecked)
				.OnCheckStateChanged(this, &SAmethystDemoHUD::OnPauseCheckStateChanged)
				[
					SNew(SImage)
					.Image(FAmethystStyle::Get().GetBrush("AmethystForest.ReplayPauseIcon"))
				]
			]

			+SVerticalBox::Slot()
		]
	];
}

FText SAmethystDemoHUD::GetCurrentReplayTime() const
{
	if (!PlayerOwner.IsValid())
	{
		return FText::GetEmpty();
	}

	auto DemoDriver = PlayerOwner->GetWorld()->DemoNetDriver;

	if (DemoDriver == nullptr)
	{
		return FText::GetEmpty();
	}

	return FText::AsTimespan(FTimespan(0, 0, static_cast<int32>(DemoDriver->DemoCurrentTime)));
}

FText SAmethystDemoHUD::GetTotalReplayTime() const
{
	if (!PlayerOwner.IsValid())
	{
		return FText::GetEmpty();
	}

	auto DemoDriver = PlayerOwner->GetWorld()->DemoNetDriver;

	if (DemoDriver == nullptr)
	{
		return FText::GetEmpty();
	}

	return FText::AsTimespan(FTimespan(0, 0, static_cast<int32>(DemoDriver->DemoTotalTime)));
}

FText SAmethystDemoHUD::GetPlaybackSpeed() const
{
	if (!PlayerOwner.IsValid() || PlayerOwner->GetWorldSettings() == nullptr)
	{
		return FText::GetEmpty();
	}

	if (PlayerOwner->GetWorldSettings()->Pauser == nullptr)
	{
		FNumberFormattingOptions FormatOptions = FNumberFormattingOptions()
			.SetMinimumFractionalDigits(2)
			.SetMaximumFractionalDigits(2);

		return FText::Format(NSLOCTEXT("AmethystForest.HUD.Menu", "PlaybackSpeed", "{0} X"), FText::AsNumber(PlayerOwner->GetWorldSettings()->DemoPlayTimeDilation, &FormatOptions));
	}

	return NSLOCTEXT("AmethystForest.HUD.Menu", "Paused", "PAUSED");
}

ECheckBoxState SAmethystDemoHUD::IsPauseChecked() const
{
	if (PlayerOwner.IsValid() && PlayerOwner->GetWorldSettings() != nullptr && PlayerOwner->GetWorldSettings()->Pauser != nullptr)
	{
		return ECheckBoxState::Checked;
	}

	return ECheckBoxState::Unchecked;
}

void SAmethystDemoHUD::OnPauseCheckStateChanged(ECheckBoxState CheckState) const
{
	if (!PlayerOwner.IsValid())
	{
		return;
	}

	AWorldSettings* WorldSettings = PlayerOwner->GetWorldSettings();

	if (WorldSettings == nullptr)
	{
		return;
	}

	switch(CheckState)
	{
		case ECheckBoxState::Checked:
		{
			WorldSettings->Pauser = PlayerOwner->PlayerState;
			break;
		}

		case ECheckBoxState::Unchecked:
		{
			WorldSettings->Pauser = nullptr;
			break;
		}

		case ECheckBoxState::Undetermined:
		{
			break;
		}
	}
}