// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehicle.h"
//#include "PIDController.h"
#include "QuickStartPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UTextRenderComponent;
class UInputComponent;

UCLASS(config=Game)
class AQuickStartPawn : public AWheeledVehicle
{
	GENERATED_BODY()

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	/** SCene component for the In-Car view origin */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* InternalCameraBase;

	/** Camera component for the In-Car view */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* InternalCamera;

	/** Text component for the In-Car speed */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* InCarSpeed;

	/** Text component for the In-Car gear */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* InCarGear;
		
	uint32 TickCounter;
	float TimeIdx;
	float VelocityError;
	float SUMoCurrVel;
	float SUMoCurrTime;

	FRotator VehicleRotation;
	float DesAngle;
	float PreviousYaw;
	float CurrentYaw;
	float DesiredYaw;

	float IntegratorError;
	float ProportionalError;
	float DerivativeError;
	float TotalLongitudinalError;
	float GainFactor;

	float RightGain;

	float ThrottleVal;
	float SteeringVal;

	float const EarthRadius = 6367.5*1000*100; // in centimeters

	void DriverModel(float DeltaTime);
	/*PIDController* PIDLat;
	PIDController* PIDLong;*/
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//float GetYawRate(float DeltaTime);
	//float GetDesiredYawRate(float DeltaTime);
	//float GetCrosstrackError();
	TArray<float> GetNextSUMoData(float DeltaTime);

	TArray<FString> VehicleDataOut;
	TArray<FString> VehicleDataIn;

	//FVector DesiredPosition;
	//FVector NextDesiredPosition;
	//FVector PositionError;
	FVector DesVel;
	float YawError;

	//FVector StartingFwdVector;
	float StartingAngle;

	float HeadingError;
	float IntegralHeadingError;
	float DerivativeHeadingError;

	FVector FinalDirection;	

public:
	AQuickStartPawn();

	/** The current speed as a string eg 10 km/h */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText SpeedDisplayString;

	/** The current gear as a string (R,N, 1,2 etc) */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText GearDisplayString;

	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	/** The color of the incar gear text in forward gears */
	FColor	GearDisplayColor;

	/** The color of the incar gear text when in reverse */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FColor	GearDisplayReverseColor;

	/** Are we using incar camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bInCarCameraActive;

	/** Are we in reverse gear */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bInReverseGear;

	/** Initial offset of incar camera */
	FVector InternalCameraOrigin;
	// Begin Pawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End Pawn interface

	// Begin Actor interface
	virtual void Tick(float Delta) override;

	// For setting desired vehicle position, so that the driver model can move the vehicle there.
	void SetDesiredVeloctiy(FVector DesiredVelocity);

	UPROPERTY(EditAnywhere)
	float KPLong;
	
	UPROPERTY(EditAnywhere)
	float KILong;
	
	UPROPERTY(EditAnywhere)
	float KDLong;

	UPROPERTY(EditAnywhere)
	float ThrottleFactor;
	
	UPROPERTY(EditAnywhere)
	float KPLat;

	/*UPROPERTY(EditAnywhere)
	float KSoft;*/

	UPROPERTY(EditAnywhere)
	float KILat;

	UPROPERTY(EditAnywhere)
	float KDLat;

	/*UPROPERTY(EditAnywhere)
	float KNonLinear;*/

	UPROPERTY(EditAnywhere)
	float FinalVel;

	UPROPERTY(EditAnywhere)
	float RampSlope;

	UPROPERTY(EditAnywhere)
	float TurnAngle;

	UPROPERTY(EditAnywhere)
	uint32 TurnStart = 400;

	UPROPERTY(EditAnywhere)
	uint32 TurnDuration = 300;

protected:
	virtual void BeginPlay() override;

public:
	// End Actor interface

	/** Handle pressing forwards */
	UFUNCTION(BlueprintCallable, DisplayName = "Forward input")
	void MoveForward(float Val);
	/** Setup the strings used on the hud */
	void SetupInCarHUD();

	/** Update the physics material used by the vehicle mesh */
	void UpdatePhysicsMaterial();
	/** Handle pressing right */
	UFUNCTION(BlueprintCallable, DisplayName = "Right input")
	void MoveRight(float Val);
	/** Handle handbrake pressed */
	void OnHandbrakePressed();
	/** Handle handbrake released */
	void OnHandbrakeReleased();
	/** Switch between cameras */
	void OnToggleCamera();
	/** Handle reset VR device */
	void OnResetVR();

	static const FName LookUpBinding;
	static const FName LookRightBinding;

private:
	/** 
	 * Activate In-Car camera. Enable camera and sets visibility of incar hud display
	 *
	 * @param	bState true will enable in car view and set visibility of various if its doesnt match new state
	 * @param	bForce true will force to always change state
	 */
	void EnableIncarView( const bool bState, const bool bForce = false );

	/** Update the gear and speed strings */
	void UpdateHUDStrings();

	/* Are we on a 'slippery' surface */
	bool bIsLowFriction;


public:
	/** Returns SpringArm subobject **/
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	/** Returns InternalCamera subobject **/
	FORCEINLINE UCameraComponent* GetInternalCamera() const { return InternalCamera; }
	/** Returns InCarSpeed subobject **/
	FORCEINLINE UTextRenderComponent* GetInCarSpeed() const { return InCarSpeed; }
	/** Returns InCarGear subobject **/
	FORCEINLINE UTextRenderComponent* GetInCarGear() const { return InCarGear; }
};
