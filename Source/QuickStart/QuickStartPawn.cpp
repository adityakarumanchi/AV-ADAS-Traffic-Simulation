// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "QuickStartPawn.h"
#include "QuickStartWheelFront.h"
#include "QuickStartWheelRear.h"
#include "QuickStartHud.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Materials/Material.h"
#include "GameFramework/Controller.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "FileHelper.h"
#include "DrawDebugHelpers.h"

#ifndef HMD_MODULE_INCLUDED
#define HMD_MODULE_INCLUDED 0
#endif

// Needed for VR Headset
#if HMD_MODULE_INCLUDED
#include "IXRTrackingSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#endif // HMD_MODULE_INCLUDED

const FName AQuickStartPawn::LookUpBinding("LookUp");
const FName AQuickStartPawn::LookRightBinding("LookRight");

#define LOCTEXT_NAMESPACE "VehiclePawn"

AQuickStartPawn::AQuickStartPawn()
{
	// Car mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/Vehicle/Sedan/Sedan_SkelMesh.Sedan_SkelMesh"));
	GetMesh()->SetSkeletalMesh(CarMesh.Object);

	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/Vehicle/Sedan/Sedan_AnimBP"));
	GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);
	
	// Simulation
	UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

	check(Vehicle4W->WheelSetups.Num() == 4);

	Vehicle4W->WheelSetups[0].WheelClass = UQuickStartWheelFront::StaticClass();
	Vehicle4W->WheelSetups[0].BoneName = FName("Wheel_Front_Left");
	Vehicle4W->WheelSetups[0].AdditionalOffset = FVector(0.f, -12.f, 0.f);

	Vehicle4W->WheelSetups[1].WheelClass = UQuickStartWheelFront::StaticClass();
	Vehicle4W->WheelSetups[1].BoneName = FName("Wheel_Front_Right");
	Vehicle4W->WheelSetups[1].AdditionalOffset = FVector(0.f, 12.f, 0.f);

	Vehicle4W->WheelSetups[2].WheelClass = UQuickStartWheelRear::StaticClass();
	Vehicle4W->WheelSetups[2].BoneName = FName("Wheel_Rear_Left");
	Vehicle4W->WheelSetups[2].AdditionalOffset = FVector(0.f, -12.f, 0.f);

	Vehicle4W->WheelSetups[3].WheelClass = UQuickStartWheelRear::StaticClass();
	Vehicle4W->WheelSetups[3].BoneName = FName("Wheel_Rear_Right");
	Vehicle4W->WheelSetups[3].AdditionalOffset = FVector(0.f, 12.f, 0.f);

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->TargetOffset = FVector(0.f, 0.f, 200.f);
	SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	// Create In-Car camera component 
	InternalCameraOrigin = FVector(0.0f, -40.0f, 120.0f);

	InternalCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("InternalCameraBase"));
	InternalCameraBase->SetRelativeLocation(InternalCameraOrigin);
	InternalCameraBase->SetupAttachment(GetMesh());

	InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
	InternalCamera->bUsePawnControlRotation = false;
	InternalCamera->FieldOfView = 90.f;
	InternalCamera->SetupAttachment(InternalCameraBase);

	//Setup TextRenderMaterial
	static ConstructorHelpers::FObjectFinder<UMaterial> TextMaterial(TEXT("Material'/Engine/EngineMaterials/AntiAliasedTextMaterialTranslucent.AntiAliasedTextMaterialTranslucent'"));
	
	UMaterialInterface* Material = TextMaterial.Object;

	// Create text render component for in car speed display
	InCarSpeed = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarSpeed"));
	InCarSpeed->SetTextMaterial(Material);
	InCarSpeed->SetRelativeLocation(FVector(70.0f, -75.0f, 99.0f));
	InCarSpeed->SetRelativeRotation(FRotator(18.0f, 180.0f, 0.0f));
	InCarSpeed->SetupAttachment(GetMesh());
	InCarSpeed->SetRelativeScale3D(FVector(1.0f, 0.4f, 0.4f));

	// Create text render component for in car gear display
	InCarGear = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
	InCarGear->SetTextMaterial(Material);
	InCarGear->SetRelativeLocation(FVector(66.0f, -9.0f, 95.0f));	
	InCarGear->SetRelativeRotation(FRotator(25.0f, 180.0f,0.0f));
	InCarGear->SetRelativeScale3D(FVector(1.0f, 0.4f, 0.4f));
	InCarGear->SetupAttachment(GetMesh());
	
	// Colors for the incar gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	// Colors for the in-car gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	bInReverseGear = false;

	// Additional stuff for automated movement
	FString LoadFileName = FString(TEXT("D:/UE4_projects/VehicleSpeedV0.csv"));
	bool LoadResult = FFileHelper::LoadFileToStringArray(VehicleDataIn, *LoadFileName);

	if (LoadResult)
	{
		UE_LOG(LogTemp, Display, TEXT("Load successful"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Load failed"));
	}
	VehicleDataIn.RemoveAt(0); // To remove header with column labels

	SUMoCurrVel = 0;
	SUMoCurrTime = 0;

	IntegratorError = 0;
	ProportionalError = 0;
	DerivativeError = 0;
	TotalLongitudinalError = 0;
	YawError = 0;

	KPLong = 2;
	KILong = 0.1;
	KDLong = 0.001;

	KPLat = 0.05;
	//KSoft = 1;
	KILat = 0.001;
	KDLat = 0;
	//KNonLinear = 1;

	ThrottleFactor = 1;
	//RightGain = 0.01;
	TickCounter = 0;
	TimeIdx = 0;

	FinalVel = 3200;
	RampSlope = 5;

	CurrentYaw = GetActorRotation().Yaw;
	PreviousYaw = CurrentYaw;

	TurnAngle = 90;
	HeadingError = 0;
	IntegralHeadingError = 0;
	DerivativeHeadingError = 0;
}

void AQuickStartPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AQuickStartPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AQuickStartPawn::MoveRight);
	PlayerInputComponent->BindAxis("LookUp");
	PlayerInputComponent->BindAxis("LookRight");

	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &AQuickStartPawn::OnHandbrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &AQuickStartPawn::OnHandbrakeReleased);
	PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &AQuickStartPawn::OnToggleCamera);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AQuickStartPawn::OnResetVR); 
}

void AQuickStartPawn::MoveForward(float Val)
{
	GetVehicleMovementComponent()->SetThrottleInput(Val);	
}

void AQuickStartPawn::MoveRight(float Val)
{
	//UE_LOG(LogTemp, Display, TEXT("Steering input: %s"), *FString::SanitizeFloat(Val));
	GetVehicleMovementComponent()->SetSteeringInput(Val);	
}

void AQuickStartPawn::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void AQuickStartPawn::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void AQuickStartPawn::OnToggleCamera()
{
	EnableIncarView(!bInCarCameraActive);
}

void AQuickStartPawn::EnableIncarView(const bool bState, const bool bForce)
{
	if ((bState != bInCarCameraActive) || ( bForce == true ))
	{
		bInCarCameraActive = bState;
		
		if (bState == true)
		{
			OnResetVR();
			Camera->Deactivate();
			InternalCamera->Activate();
		}
		else
		{
			InternalCamera->Deactivate();
			Camera->Activate();
		}
		
		InCarSpeed->SetVisibility(bInCarCameraActive);
		InCarGear->SetVisibility(bInCarCameraActive);
	}
}


void AQuickStartPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// Setup the flag to say we are in reverse gear
	bInReverseGear = GetVehicleMovement()->GetCurrentGear() < 0;
	
	// Update the strings used in the hud (incar and onscreen)
	UpdateHUDStrings();

	// Set the string in the incar hud
	SetupInCarHUD();

	bool bHMDActive = false;
#if HMD_MODULE_INCLUDED
	if ((GEngine->XRSystem.IsValid() == true) && ((GEngine->XRSystem->IsHeadTrackingAllowed() == true) || (GEngine->IsStereoscopic3D() == true)))
	{
		bHMDActive = true;
	}
#endif // HMD_MODULE_INCLUDED
	if (bHMDActive == false)
	{
		if ( (InputComponent) && (bInCarCameraActive == true ))
		{
			FRotator HeadRotation = InternalCamera->RelativeRotation;
			HeadRotation.Pitch += InputComponent->GetAxisValue(LookUpBinding);
			HeadRotation.Yaw += InputComponent->GetAxisValue(LookRightBinding);
			InternalCamera->RelativeRotation = HeadRotation;
		}
	}

	// Additional stuff	
	//UE_LOG(LogTemp, Display, TEXT("%s"), *VehicleDataIn[TickCounter]);

	// Velocity Control
	if (TickCounter == 0)
	{
		//PreviousYaw = CurrentYaw;
		//StartingFwdVector = GetActorForwardVector();
		StartingAngle = GetActorRotation().Yaw;
		//FinalDirection = StartingFwdVector.RotateAngleAxis(-TurnAngle, FVector(0, 0, 1));

	}
	
	TArray<float> SUMoData = GetNextSUMoData(Delta);
	if (SUMoData.Num() > 0)
	{
		//DesVel = (SUMoData[2] / ((SUMoData[0] - TimeIdx) / Delta)) * GetActorForwardVector(); // - GetVelocity().Size()
		//DesVel = (SUMoCurrVel + ((SUMoData[2] - SUMoCurrVel) * ((SUMoCurrTime - TimeIdx)/Delta)) / (SUMoData[0] - SUMoCurrTime)) * GetActorForwardVector();
		//DesVel = ((GetVelocity().Size() + SUMoData[2]) / 2)*GetActorForwardVector();
		//DesVel = ((SUMoCurrVel + GetVelocity().Size() + SUMoData[2]) / 3)*GetActorForwardVector();
		DesVel = SUMoData[2] * GetActorForwardVector();		
		UE_LOG(LogTemp, Display, TEXT("SUMoVel.: %s, SUMoCurrVel: %s, Des Vel.: %s, SUMo time: %s, TimeIdx: %s"), *FString::SanitizeFloat(SUMoData[2]),
			*FString::SanitizeFloat(SUMoCurrVel), *FString::SanitizeFloat(DesVel.Size()),
			*FString::SanitizeFloat(SUMoData[0]), *FString::SanitizeFloat(TimeIdx));
		DesAngle = SUMoData[1];
		
		// Not a general solution, applicable for this specific file. Need to implement properly based on SUMo to UE coordinate transform
		if (DesAngle != 0)
		{
			DesAngle -= 360;
		}
	}

	/*UE_LOG(LogTemp, Display, TEXT("DesAngle: %s, DesiredYaw: %s, CurrentYaw: %s"), *FString::SanitizeFloat(DesAngle),
		*FString::SanitizeFloat(DesiredYaw), *FString::SanitizeFloat(CurrentYaw));*/
	//UE_LOG(LogTemp, Display, TEXT("Current Quat: %s"), *GetActorQuat().ToString());
		
	//UE_LOG(LogTemp, Display, TEXT("DesVel: %s"), *DesVel.ToString());
	SetDesiredVeloctiy(DesVel);
	DriverModel(Delta);
	DrawDebugPoint(GetWorld(), GetActorLocation(), 5, FColor::Purple, true, 0.03, 0);

	/*if (TickCounter <= FinalVel/RampSlope)
	{
		DesVel = TickCounter * RampSlope * GetActorForwardVector();
	}
	else
	{
		DesVel =  FinalVel * GetActorForwardVector();
	}*/

	// Turning Control
	CurrentYaw = GetActorRotation().Yaw;
	DesiredYaw = StartingAngle - DesAngle;
	
	//if (TickCounter <= TurnStart)
	//{
	//	DesiredYaw = CurrentYaw;
	//}
	//else if (TickCounter > TurnStart && TickCounter <= TurnStart+TurnDuration)
	//{
	//	UE_LOG(LogTemp, Display, TEXT("TickCounter: %s"), *FString::FromInt(TickCounter));		
	//	DesiredYaw = StartingAngle + TurnAngle * (TickCounter - TurnStart) / TurnDuration;
	//}
	//else
	//{		
	//	DesiredYaw = StartingAngle + TurnAngle;
	//	//NextDesiredPosition = GetActorLocation() + DesVel * Delta * 2;		
	//}
	
	VehicleDataOut.Add(FString(TEXT("Time ")) + FString::SanitizeFloat(TimeIdx) + FString(TEXT(";")) +
		FString(TEXT("Ref ")) + FString::SanitizeFloat(DesVel.Size()) + FString(TEXT(";")) +
		FString(TEXT("CurrVel ")) + FString::SanitizeFloat(GetVelocity().Size()) + FString(TEXT(";")) +
		FString(TEXT("VelErr ")) + FString::SanitizeFloat(VelocityError) + FString(TEXT(";")) +
		FString(TEXT("Throttle ")) + FString::SanitizeFloat(ThrottleVal) + FString(TEXT(";")) +
		FString(TEXT("Steering ")) + FString::SanitizeFloat(SteeringVal) + FString(TEXT(";")) +
		FString(TEXT("Yaw ")) + FString::SanitizeFloat(CurrentYaw - PreviousYaw) + FString(TEXT(";")) +
		FString(TEXT("CurrAngle ")) + FString::SanitizeFloat(GetActorRotation().Yaw) + FString(TEXT(";")) +
		FString(TEXT("Quat ")) + GetActorQuat().ToString());
		/*FString(TEXT("PError ")) + FString::SanitizeFloat(KPLat * HeadingError) + FString(TEXT(";")) +
		FString(TEXT("IError ")) + FString::SanitizeFloat(KILat * IntegralHeadingError) + FString(TEXT(";")) +
		FString(TEXT("DError ")) + FString::SanitizeFloat(KDLat * DerivativeHeadingError));*/
		
	PreviousYaw = CurrentYaw;
	TickCounter++;
	TimeIdx += Delta;
}

TArray<float> AQuickStartPawn::GetNextSUMoData(float DeltaTime)
{
	TArray<float> DataOut;
	TArray<FString> FirstRow;
	if (VehicleDataIn.Num() > 0) // for handling end of csv file
	{
		VehicleDataIn[0].ParseIntoArray(FirstRow, TEXT(","), true);
	}
	else
	{
		return DataOut; // DataOut will be empty in this case
	}
	
	//VelocityAndAngle.Init(FCString::Atof(*FirstRow[7]), FCString::Atof(*FirstRow[8]));
	if (TimeIdx + DeltaTime >= FCString::Atof(*FirstRow[1]))
	{
		SUMoCurrVel = 100 * FCString::Atof(*FirstRow[8]);
		SUMoCurrTime = FCString::Atof(*FirstRow[1]);
		VehicleDataIn.RemoveAt(0);
	}
	else
	{
		// [1] is SUMo time, [7] is angle, [8] is speed. Factor of 100 to convert from m/s to cm/s
		//DataOut.Init(FCString::Atof(*FirstRow[1]), FCString::Atof(*FirstRow[7]), (100 * FCString::Atof(*FirstRow[8])));
		DataOut.Push(FCString::Atof(*FirstRow[1]));
		DataOut.Push(FCString::Atof(*FirstRow[7]));
		DataOut.Push(100 * FCString::Atof(*FirstRow[8]));
	}
	//for (auto d : DataOut)//FirstRow)
	//{
	//	UE_LOG(LogTemp, Display, TEXT("%s"), *FString::SanitizeFloat(d));
	//}
	//UE_LOG(LogTemp, Display, TEXT("Done"));
	
	return DataOut;
}

void AQuickStartPawn::SetDesiredVeloctiy(FVector DesiredVelocity)
{
	GainFactor = DesiredVelocity.Size();
	//GainFactor = 1;
	VelocityError = ((DesiredVelocity - GetVelocity()).Size())*(GetActorForwardVector().CosineAngle2D(DesiredVelocity - GetVelocity()));	
}

void AQuickStartPawn::DriverModel(float DeltaTime)
{	
	//if (FGenericPlatformMath::Abs(TotalLongitudinalError)*ThrottleFactor < 1)
	if (FGenericPlatformMath::Abs(ThrottleVal) < 1)
	{
		IntegratorError += VelocityError * DeltaTime; //Has to be multiplied by dt, coz integral error should be area under error curve, not just sum of error.
		DerivativeError -= ProportionalError / DeltaTime;
	}	
	
	ProportionalError = VelocityError;
		
	TotalLongitudinalError = (KPLong / GainFactor) * ProportionalError + (KILong / GainFactor) * IntegratorError - (KDLong / GainFactor) * DerivativeError;
	ThrottleVal = ThrottleFactor * TotalLongitudinalError;
	if (FGenericPlatformMath::Abs(VelocityError) > 25)
	{
		MoveForward(ThrottleVal);
	}

	/*if (SteeringVal<=1)
		YawError += CurrentYaw - DesiredYaw;*/
	//YawError = CurrentYaw - DesiredYaw;

	//SteeringVal = KIYaw*YawError + KNonLinear*FGenericPlatformMath::Atan2(KLat*GetCrosstrackError(), KSoft + GetVelocity().Size())
		//+ KDYaw * (GetYawRate(DeltaTime) - GetDesiredYawRate(DeltaTime));

		DerivativeHeadingError -= HeadingError / DeltaTime;
		HeadingError = CurrentYaw - DesiredYaw;

	if (FGenericPlatformMath::Abs(SteeringVal) < 1)
	{
		IntegralHeadingError += HeadingError * DeltaTime;
	}
	
	SteeringVal = KPLat * HeadingError + KILat * IntegralHeadingError - KDLat * DerivativeHeadingError;

	if (FGenericPlatformMath::Abs(SteeringVal) >= 0.001)
		MoveRight(-SteeringVal);
	else
		MoveRight(0);
}

void AQuickStartPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	FString SaveName = FString(TEXT("D:/UE4_projects/VehicleData.csv"));
	FFileHelper::SaveStringArrayToFile(VehicleDataOut, *SaveName);
}

void AQuickStartPawn::BeginPlay()
{
	Super::BeginPlay();

	bool bEnableInCar = false;
#if HMD_MODULE_INCLUDED
	bEnableInCar = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
#endif // HMD_MODULE_INCLUDED
	EnableIncarView(bEnableInCar,true);
}

void AQuickStartPawn::OnResetVR()
{
#if HMD_MODULE_INCLUDED
	if (GEngine->XRSystem.IsValid())
	{
		GEngine->XRSystem->ResetOrientationAndPosition();
		InternalCamera->SetRelativeLocation(InternalCameraOrigin);
		GetController()->SetControlRotation(FRotator());
	}
#endif // HMD_MODULE_INCLUDED
}

void AQuickStartPawn::UpdateHUDStrings()
{
	float KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
	int32 KPH_int = FMath::FloorToInt(KPH);

	// Using FText because this is display text that should be localizable
	SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0} km/h"), FText::AsNumber(KPH_int));
	
	if (bInReverseGear == true)
	{
		GearDisplayString = FText(LOCTEXT("ReverseGear", "R"));
	}
	else
	{
		int32 Gear = GetVehicleMovement()->GetCurrentGear();
		GearDisplayString = (Gear == 0) ? LOCTEXT("N", "N") : FText::AsNumber(Gear);
	}	
}

void AQuickStartPawn::SetupInCarHUD()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if ((PlayerController != nullptr) && (InCarSpeed != nullptr) && (InCarGear != nullptr) )
	{
		// Setup the text render component strings
		InCarSpeed->SetText(SpeedDisplayString);
		InCarGear->SetText(GearDisplayString);
		
		if (bInReverseGear == false)
		{
			InCarGear->SetTextRenderColor(GearDisplayColor);
		}
		else
		{
			InCarGear->SetTextRenderColor(GearDisplayReverseColor);
		}
	}
}

//float AQuickStartPawn::GetYawRate(float DeltaTime)
//{	
//	float YawRate = (CurrentYaw - PreviousYaw) / DeltaTime;
//	
//	return YawRate;
//}

//float AQuickStartPawn::GetDesiredYawRate(float DeltaTime)
//{
//	float ArcLength = (NextDesiredPosition - DesiredPosition).Size();
//	float DesHeadingAngle = FGenericPlatformMath::Acos(GetActorForwardVector().CosineAngle2D(NextDesiredPosition - DesiredPosition));
//	float TurnRadius = ArcLength / DesHeadingAngle;
//		
//	return (GetVelocity().Size() / TurnRadius);
//}

// Refer to "Cross-track distance" in http://www.movable-type.co.uk/scripts/latlong.html
//float AQuickStartPawn::GetCrosstrackError()
//{
//	FVector CurrentLocation = GetActorLocation();
//	float delta13 = (CurrentLocation - DesiredPosition).Size() / EarthRadius;
//	float AngleDifference = FGenericPlatformMath::Acos((CurrentLocation - DesiredPosition).CosineAngle2D(NextDesiredPosition - DesiredPosition)); // theta_13 - theta_12
//	float CrosstrackError = EarthRadius * FGenericPlatformMath::Asin(FGenericPlatformMath::Sin(delta13) * FGenericPlatformMath::Sin(AngleDifference));
//
//	return CrosstrackError;
//}

#undef LOCTEXT_NAMESPACE