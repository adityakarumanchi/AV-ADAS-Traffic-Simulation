// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "QuickStartGameMode.h"
#include "QuickStartPawn.h"
#include "QuickStartHud.h"

AQuickStartGameMode::AQuickStartGameMode()
{
	DefaultPawnClass = AQuickStartPawn::StaticClass();
	HUDClass = AQuickStartHud::StaticClass();
}
