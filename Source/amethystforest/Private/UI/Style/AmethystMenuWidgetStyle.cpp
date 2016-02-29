// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystMenuWidgetStyle.h"

FAmethystMenuStyle::FAmethystMenuStyle()
{
}

FAmethystMenuStyle::~FAmethystMenuStyle()
{
}

const FName FAmethystMenuStyle::TypeName(TEXT("FAmethystMenuStyle"));

const FAmethystMenuStyle& FAmethystMenuStyle::GetDefault()
{
	static FAmethystMenuStyle Default;
	return Default;
}

void FAmethystMenuStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	OutBrushes.Add(&HeaderBackgroundBrush);
	OutBrushes.Add(&LeftBackgroundBrush);
	OutBrushes.Add(&RightBackgroundBrush);
}


UAmethystMenuWidgetStyle::UAmethystMenuWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
