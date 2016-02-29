// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystMenuItemWidgetStyle.h"

FAmethystMenuItemStyle::FAmethystMenuItemStyle()
{
}

FAmethystMenuItemStyle::~FAmethystMenuItemStyle()
{
}

const FName FAmethystMenuItemStyle::TypeName(TEXT("FAmethystMenuItemStyle"));

const FAmethystMenuItemStyle& FAmethystMenuItemStyle::GetDefault()
{
	static FAmethystMenuItemStyle Default;
	return Default;
}

void FAmethystMenuItemStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	OutBrushes.Add(&BackgroundBrush);
	OutBrushes.Add(&LeftArrowImage);
	OutBrushes.Add(&RightArrowImage);
}


UAmethystMenuItemWidgetStyle::UAmethystMenuItemWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
