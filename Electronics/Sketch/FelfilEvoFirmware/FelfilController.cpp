// 
// 
// 

#include "FelfilController.h"

FelfilControler::FelfilControler()
{
	windowStartTime = millis();
	protectionModeActivated = false;
}

#pragma region Pin Setup

void FelfilControler::SetupPwmOutputPin(int s1Pin)
{
	this->s1Pin = s1Pin;
	pinMode(s1Pin, OUTPUT);
}

void FelfilControler::SetupTemperatureOutputPins(int s2Pin, int s3Pin, int s4Pin)
{
	this->s2Pin = s2Pin;
	this->s3Pin = s3Pin;
	this->s4Pin = s4Pin;

	pinMode(s2Pin, OUTPUT);
	pinMode(s3Pin, OUTPUT);
	pinMode(s4Pin, OUTPUT);
}

#pragma endregion

#pragma region PID Setup

void FelfilControler::SetupPidAggressiveTuning(double kp, double ki, double kd)
{
	this->aggKp = kp;
	this->aggKi = ki;
	this->aggKd = kd;
}

void FelfilControler::SetupPidConservativeTuning(double kp, double ki, double kd)
{
	this->consKp = kp;
	this->consKi = ki;
	this->consKd = kd;
}

void FelfilControler::SetupPidOuputLimits(double min, double max)
{
	pidMinValue = min;
	pidMaxValue = max;
}

void FelfilControler::InitializePid()
{
	pid = new PID(&tempInput, &tempOutput, &tempSetpoint, consKp, consKi, consKd, DIRECT);
	if (pidMinValue != 0 || pidMaxValue != 0)
		pid->SetOutputLimits(pidMinValue, pidMaxValue);
	pid->SetMode(AUTOMATIC);
}

void FelfilControler::SetPidTuning()
{
	//distance away from setpoint
	double gap = abs(tempSetpoint - tempInput);
	//we're close to setpoint, use conservative tuning parameters
	if (gap < 2)
		pid->SetTunings(consKp, consKi, consKd);
	//we're far from setpoint, use aggressive tuning parameters
	else
		pid->SetTunings(aggKp, aggKi, aggKd);
}

void FelfilControler::Initialize()
{
	InitializePid();
}

void FelfilControler::Reset()
{
	protectionModeActivated = false;
}

#pragma endregion

#pragma region Control

void FelfilControler::ControlPwm(int engineCurrent, int pwmSetpoint)
{
	if (engineCurrent >= 2)
	{
		StopEngine();
		protectionModeActivated = true;
		return;
	}

	protectionModeActivated = false;
	analogWrite(s1Pin, pwmSetpoint * 28.33);
}

void FelfilControler::StopEngine()
{
	digitalWrite(s1Pin, LOW);
}

TemperatureControlState FelfilControler::ControlTemperature(double tempInput, double tempSetpoint)
{
	this->tempInput = tempInput;
	this->tempSetpoint = tempSetpoint;

	SetPidTuning();

	//effettua il calcolo
	pid->Compute();

	//time to shift the Relay Window
	if (millis() - windowStartTime > pidMaxValue)
		windowStartTime += pidMaxValue;

	if (tempOutput < millis() - windowStartTime)
	{
		digitalWrite(s2Pin, LOW);
		digitalWrite(s3Pin, LOW);
		digitalWrite(s4Pin, LOW);
		return Low;
	}
	else
	{
		digitalWrite(s2Pin, HIGH);
		digitalWrite(s3Pin, HIGH);
		digitalWrite(s4Pin, HIGH);
		return High;
	}
}

bool FelfilControler::IsProtectionModeActivated()
{
	return protectionModeActivated;
}

#pragma endregion