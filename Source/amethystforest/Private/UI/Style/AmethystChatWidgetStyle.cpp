// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystChatWidgetStyle.h"

FAmethystChatStyle::FAmethystChatStyle()
{
}

FAmethystChatStyle::~FAmethystChatStyle()
{
}

const FName FAmethystChatStyle::TypeName(TEXT("FAmethystChatStyle"));

const FAmethystChatStyle& FAmethystChatStyle::GetDefault()
{
	static FAmethystChatStyle Default;
	return Default;
}

void FAmethystChatStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	TextEntryStyle.GetResources(OutBrushes);

	OutBrushes.Add(&BackingBrush);
}


UAmethystChatWidgetStyle::UAmethystChatWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
