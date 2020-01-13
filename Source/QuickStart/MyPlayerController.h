// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class QUICKSTART_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	//virtual void PlayerTick(float DetlaTime) override;
	virtual void Tick(float DeltaTime) override;

	void VehicleControl();

	uint32 TickCounter;
	APawn* VehPawn;
	FVector VehicleLocation;

public:
	AMyPlayerController();

	float FwdGain;
	float RightGain;
	FVector RefPosition;

protected:
	UPROPERTY(BlueprintReadOnly, DisplayName = "Forward Input")
	float FwdInput;

	UPROPERTY(BlueprintReadOnly, DisplayName = "Right Input")
	float RightInput;
};
