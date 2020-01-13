// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehicle.h"
#include "MyWheeledVehicle.generated.h"

class AMyPlayerController;
class UWheeledVehicleMovementComponent;
/**
 * 
 */
UCLASS()
class QUICKSTART_API AMyWheeledVehicle : public AWheeledVehicle
{
	GENERATED_BODY()

public:
	AMyWheeledVehicle();
	
private:	
	AMyPlayerController* MyPC;
	UWheeledVehicleMovementComponent* VehicleMC;

	void MoveForward(float Val);
	void MoveRight(float Val);
};
