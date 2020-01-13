// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "GameFramework/Pawn.h"
#include <EngineGlobals.h>
#include <Engine.h>
#include "WheeledVehicle.h"
#include "Kismet/GameplayStatics.h"

AMyPlayerController::AMyPlayerController()
{
	TickCounter = 0;
	VehPawn = GetPawn();
	//TArray<AWheeledVehicle*> Vehicles;
	//GetObjectsOfClass(AWheeledVehicle::StaticClass(), Vehicles);
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWheeledVehicle::StaticClass(), Vehicles);
	//if (VehPawn)
		

	FwdGain = 0.01f;
	RightGain = 1.0f;
	RefPosition = VehicleLocation;	
}

//void AMyPlayerController::PlayerTick(float DeltaTime)
void AMyPlayerController::Tick(float DeltaTime)
{
	TickCounter++;

	if (VehPawn)
	{
		UE_LOG(LogTemp, Display, TEXT("Pawn: %s"), *VehPawn->GetHumanReadableName());
		VehicleLocation = VehPawn->GetActorLocation();
	}
	
	//UE_LOG(LogTemp, Display, TEXT("Vehicle Location: %s"), *VehicleLocation.ToString());
	
	RefPosition.Y += 100.0f;
	//UE_LOG(LogTemp, Display, TEXT("RefPosition: %s"), *RefPosition.ToString());
	//RefPosition.Y

	VehicleControl();
	
	//if (VehPawn)
	//FVector VehVel = VehPawn->GetVelocity();

	//UE_LOG(LogTemp, Display, TEXT("Veh vel.: %s"), *VehVel.ToString());

}

void AMyPlayerController::VehicleControl()
{
	FVector PositionError = RefPosition - VehicleLocation;
	//UE_LOG(LogTemp, Display, TEXT("PositionError: %s"), *PositionError.ToString());
	FwdInput = FwdGain * PositionError.Y;
	//VehPawn->GetMovementComponent()->SetThrottleInput(FwdInput);
	//UPawnMovementComponent* VehMC = VehPawn->GetMovementComponent;
	
}
