// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "SAmethystLeaderboard.h"
#include "AmethystStyle.h"
#include "AmethystUIHelpers.h"

#if PLATFORM_XBOXONE
#define INTERACTIVE_LEADERBOARD	1
#endif

FLeaderboardRow::FLeaderboardRow(const FOnlineStatsRow& Row)
	: Rank(FString::FromInt(Row.Rank))
	, PlayerName(Row.NickName)
	, PlayerId(Row.PlayerId)
{
	if (const FVariantData* KillData = Row.Columns.Find(LEADERBOARD_STAT_KILLS))
	{
		int32 Val;
		KillData->GetValue(Val);
		Kills = FString::FromInt(Val);
	}

	if (const FVariantData* DeathData = Row.Columns.Find(LEADERBOARD_STAT_DEATHS))
	{
		int32 Val;
		DeathData->GetValue(Val);
		Deaths = FString::FromInt(Val);
	}
}

void SAmethystLeaderboard::Construct(const FArguments& InArgs)
{
	PlayerOwner = InArgs._PlayerOwner;
	OwnerWidget = InArgs._OwnerWidget;
	const int32 BoxWidth = 125;
	bReadingStats = false;

	LeaderboardReadCompleteDelegate = FOnLeaderboardReadCompleteDelegate::CreateRaw(this, &SAmethystLeaderboard::OnStatsRead);

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[		
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)  
			.WidthOverride(600)
			.HeightOverride(600)
			[
				SAssignNew(RowListWidget, SListView< TSharedPtr<FLeaderboardRow> >)
				.ItemHeight(20)
				.ListItemsSource(&StatRows)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &SAmethystLeaderboard::MakeListViewWidget)
				.OnSelectionChanged(this, &SAmethystLeaderboard::EntrySelectionChanged)
				.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Rank").FixedWidth(BoxWidth/3) .DefaultLabel(NSLOCTEXT("LeaderBoard", "PlayerRankColumn", "Rank"))
				+ SHeaderRow::Column("PlayerName").FixedWidth(BoxWidth*2) .DefaultLabel(NSLOCTEXT("LeaderBoard", "PlayerNameColumn", "Player Name"))
				+ SHeaderRow::Column("Kills") .DefaultLabel(NSLOCTEXT("LeaderBoard", "KillsColumn", "Kills"))
				+ SHeaderRow::Column("Deaths") .DefaultLabel(NSLOCTEXT("LeaderBoard", "DeathsColumn", "Deaths")))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		[
			SNew(STextBlock)
			.Text(AmethystUIHelpers::Get().GetProfileOpenText())
			.TextStyle(FAmethystStyle::Get(), "AmethystForest.ScoreboardListTextStyle")
			.Visibility(this, &SAmethystLeaderboard::GetProfileUIVisibility)
		]
	];
}

/** Starts reading leaderboards for the game */
void SAmethystLeaderboard::ReadStats()
{
	StatRows.Reset();

	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
		if (Leaderboards.IsValid())
		{
			// We are about to read the stats. The delegate will set this to false once the read is complete.
			LeaderboardReadCompleteDelegateHandle = Leaderboards->AddOnLeaderboardReadCompleteDelegate_Handle(LeaderboardReadCompleteDelegate);
			bReadingStats = true;

			// There's no reason to request leaderboard requests while one is in progress, so only do it if there isn't one active.
			if (!IsLeaderboardReadInProgress())
			{
				ReadObject = MakeShareable(new FAmethystAllTimeMatchResultsRead());
				FOnlineLeaderboardReadRef ReadObjectRef = ReadObject.ToSharedRef();
				bReadingStats = Leaderboards->ReadLeaderboardsForFriends(0, ReadObjectRef);
			}
		}
		else
		{
			// TODO: message the user?
		}
	}
}

/** Called on a particular leaderboard read */
void SAmethystLeaderboard::OnStatsRead(bool bWasSuccessful)
{
	// It's possible for another read request to happen while another one is ongoing (such as when the player leaves the menu and
	// re-enters quickly). We only want to process a leaderboard read if our read object is done.
	if (!IsLeaderboardReadInProgress())
	{
		ClearOnLeaderboardReadCompleteDelegate();

		if (bWasSuccessful)
		{
			for (int Idx = 0; Idx < ReadObject->Rows.Num(); ++Idx)
			{
				TSharedPtr<FLeaderboardRow> NewLeaderboardRow = MakeShareable(new FLeaderboardRow(ReadObject->Rows[Idx]));

				StatRows.Add(NewLeaderboardRow);
			}

			RowListWidget->RequestListRefresh();
		}

		bReadingStats = false;
	}
}

void SAmethystLeaderboard::OnFocusLost( const FFocusEvent& InFocusEvent )
{
	if (InFocusEvent.GetCause() != EFocusCause::SetDirectly)
	{
		FSlateApplication::Get().SetKeyboardFocus(SharedThis( this ));
	}
}

FReply SAmethystLeaderboard::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	return FReply::Handled().SetUserFocus(RowListWidget.ToSharedRef(), EFocusCause::SetDirectly);
}

void SAmethystLeaderboard::EntrySelectionChanged(TSharedPtr<FLeaderboardRow> InItem, ESelectInfo::Type SelectInfo)
{
	SelectedItem = InItem;
}

bool SAmethystLeaderboard::IsPlayerSelectedAndValid() const
{
#if INTERACTIVE_LEADERBOARD
	if (SelectedItem.IsValid())
	{
		check(SelectedItem->PlayerId.IsValid());
		return true;
	}
#endif
	return false;
}

EVisibility SAmethystLeaderboard::GetProfileUIVisibility() const
{	
	return IsPlayerSelectedAndValid() ? EVisibility::Visible : EVisibility::Hidden;
}

bool SAmethystLeaderboard::ProfileUIOpened() const
{
	if( IsPlayerSelectedAndValid() )
	{
		check( PlayerOwner.IsValid() );
		const TSharedPtr<const FUniqueNetId> OwnerNetId = PlayerOwner->GetPreferredUniqueNetId();
		check( OwnerNetId.IsValid() );

		const TSharedPtr<const FUniqueNetId>& PlayerId = SelectedItem->PlayerId;
		check( PlayerId.IsValid() );
		return AmethystUIHelpers::Get().ProfileOpenedUI(*OwnerNetId.Get(), *PlayerId.Get(), NULL);
	}
	return false;
}

void SAmethystLeaderboard::MoveSelection(int32 MoveBy)
{
	int32 SelectedItemIndex = StatRows.IndexOfByKey(SelectedItem);

	if (SelectedItemIndex+MoveBy > -1 && SelectedItemIndex+MoveBy < StatRows.Num())
	{
		RowListWidget->SetSelection(StatRows[SelectedItemIndex+MoveBy]);
	}
}

FReply SAmethystLeaderboard::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	FReply Result = FReply::Unhandled();
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Up || Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_LeftStick_Up)
	{
		MoveSelection(-1);
		Result = FReply::Handled();
	}
	else if (Key == EKeys::Down || Key == EKeys::Gamepad_DPad_Down || Key == EKeys::Gamepad_LeftStick_Down)
	{
		MoveSelection(1);
		Result = FReply::Handled();
	}
	else if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Gamepad_Special_Left)
	{
		if (bReadingStats)
		{
			ClearOnLeaderboardReadCompleteDelegate();
			bReadingStats = false;
		}
	}
	else if (Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		// Open the profile UI of the selected item
		ProfileUIOpened();
		Result = FReply::Handled();
		FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	}
	return Result;
}

TSharedRef<ITableRow> SAmethystLeaderboard::MakeListViewWidget(TSharedPtr<FLeaderboardRow> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	class SLeaderboardRowWidget : public SMultiColumnTableRow< TSharedPtr<FLeaderboardRow> >
	{
	public:
		SLATE_BEGIN_ARGS(SLeaderboardRowWidget){}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FLeaderboardRow> InItem)
		{
			Item = InItem;
			SMultiColumnTableRow< TSharedPtr<FLeaderboardRow> >::Construct(FSuperRowType::FArguments(), InOwnerTable);
		}

		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName)
		{
			FText ItemText = FText::GetEmpty();
			if (ColumnName == "Rank")
			{
				ItemText = FText::FromString(Item->Rank);
			}
			else if (ColumnName == "PlayerName")
			{
				if (Item->PlayerName.Len() > MAX_PLAYER_NAME_LENGTH)
				{
					ItemText = FText::FromString(Item->PlayerName.Left(MAX_PLAYER_NAME_LENGTH) + "...");
				}
				else
				{
					ItemText = FText::FromString(Item->PlayerName);
				}
			}
			else if (ColumnName == "Kills")
			{
				ItemText = FText::FromString(Item->Kills);
			}
			else if (ColumnName == "Deaths")
			{
				ItemText = FText::FromString(Item->Deaths);
			}
			return SNew(STextBlock)
				.Text(ItemText)
				.TextStyle(FAmethystStyle::Get(), "AmethystForest.ScoreboardListTextStyle");
		}
		TSharedPtr<FLeaderboardRow> Item;
	};
	return SNew(SLeaderboardRowWidget, OwnerTable, Item);
}

void SAmethystLeaderboard::ClearOnLeaderboardReadCompleteDelegate()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
		if (Leaderboards.IsValid())
		{
			Leaderboards->ClearOnLeaderboardReadCompleteDelegate_Handle(LeaderboardReadCompleteDelegateHandle);
		}
	}
}

bool SAmethystLeaderboard::IsLeaderboardReadInProgress()
{
	return ReadObject.IsValid() && (ReadObject->ReadState == EOnlineAsyncTaskState::InProgress || ReadObject->ReadState == EOnlineAsyncTaskState::NotStarted);
}