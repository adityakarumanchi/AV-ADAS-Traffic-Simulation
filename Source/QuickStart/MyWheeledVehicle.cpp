// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWheeledVehicle.h"
#include "WheeledVehicleMovementComponent.h"
#include "MyPlayerController.h"

AMyWheeledVehicle::AMyWheeledVehicle()
{
	MyPC = Cast<AMyPlayerController>(GetController());
	VehicleMC = GetVehicleMovementComponent();
}

void AMyWheeledVehicle::MoveForward(float Val)
{
	if (VehicleMC)
	{
		VehicleMC->SetThrottleInput(Val);
	}
}

void AMyWheeledVehicle::MoveRight(float Val)
{
	if (VehicleMC)
	{
		VehicleMC->SetSteeringInput(Val);
	}
}