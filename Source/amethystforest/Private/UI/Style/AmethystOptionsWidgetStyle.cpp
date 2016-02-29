// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AmethystForest.h"
#include "AmethystOptionsWidgetStyle.h"

FAmethystOptionsStyle::FAmethystOptionsStyle()
{
}

FAmethystOptionsStyle::~FAmethystOptionsStyle()
{
}

const FName FAmethystOptionsStyle::TypeName(TEXT("FAmethystOptionsStyle"));

const FAmethystOptionsStyle& FAmethystOptionsStyle::GetDefault()
{
	static FAmethystOptionsStyle Default;
	return Default;
}

void FAmethystOptionsStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
}


UAmethystOptionsWidgetStyle::UAmethystOptionsWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
