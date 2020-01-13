// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "QuickStartHud.generated.h"


UCLASS(config = Game)
class AQuickStartHud : public AHUD
{
	GENERATED_BODY()

public:
	AQuickStartHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
