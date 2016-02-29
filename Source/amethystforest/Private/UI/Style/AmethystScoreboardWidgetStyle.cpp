// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystScoreboardWidgetStyle.h"

FAmethystScoreboardStyle::FAmethystScoreboardStyle()
{
}

FAmethystScoreboardStyle::~FAmethystScoreboardStyle()
{
}

const FName FAmethystScoreboardStyle::TypeName(TEXT("FAmethystScoreboardStyle"));

const FAmethystScoreboardStyle& FAmethystScoreboardStyle::GetDefault()
{
	static FAmethystScoreboardStyle Default;
	return Default;
}

void FAmethystScoreboardStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	OutBrushes.Add(&ItemBorderBrush);
}


UAmethystScoreboardWidgetStyle::UAmethystScoreboardWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
