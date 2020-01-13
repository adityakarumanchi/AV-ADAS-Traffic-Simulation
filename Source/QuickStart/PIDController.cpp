// Fill out your copyright notice in the Description page of Project Settings.


#include "PIDController.h"


PIDController::PIDController(float pFactor, float iFactor, float dFactor)
{
	this->pFactor = pFactor;
	this->iFactor = iFactor;
	this->dFactor = dFactor;
	lastError = 0;
	ki = 0;
	kp = 0;
	kd = 0;
}

PIDController::~PIDController()
{
}

float PIDController::Update(float Error, float time)
{
	float currentError = Error;
	if (currentError != 0)
	{
		kp = currentError * pFactor;
		ki = iFactor + currentError * pFactor;
		kd = (currentError - lastError) * dFactor;
	}
	else
	{
		ki = 0;
		kp = 0;
		kd = 0;
	}

	lastError = currentError;
	return kp + ki + kd;
}