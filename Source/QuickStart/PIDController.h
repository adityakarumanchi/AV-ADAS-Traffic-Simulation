// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"

/**
 * 
 */
class QUICKSTART_API PIDController
{
public:

	UPROPERTY(EditAnywhere)
	float pFactor, iFactor, dFactor; //Propotional, Integral, Derivative Factors

private:
	float kp, ki, kd; // Constants
	float lastError; // Used in the kd equation to determine PI gain
	float integralActiveZone = 50; // Integral maximum value

public:
	PIDController(float pFactor, float iFactor, float dFactor);
	~PIDController();
	float Update(float Error, float time);
};
