// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystMenuSoundsWidgetStyle.h"

FAmethystMenuSoundsStyle::FAmethystMenuSoundsStyle()
{
}

FAmethystMenuSoundsStyle::~FAmethystMenuSoundsStyle()
{
}

const FName FAmethystMenuSoundsStyle::TypeName(TEXT("FAmethystMenuSoundsStyle"));

const FAmethystMenuSoundsStyle& FAmethystMenuSoundsStyle::GetDefault()
{
	static FAmethystMenuSoundsStyle Default;
	return Default;
}

void FAmethystMenuSoundsStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
}


UAmethystMenuSoundsWidgetStyle::UAmethystMenuSoundsWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
