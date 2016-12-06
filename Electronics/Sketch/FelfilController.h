// FelfilController.h

#ifndef _FELFILCONTROLLER_h
#define _FELFILCONTROLLER_h

#include <PID_v1.h>
//#include "FakePid.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

enum TemperatureControlState
{
	Low = 0,
	High = 1
};

class FelfilControler
{
private:
	PID* pid;
	
	//attivazione modalità di protezione
	bool protectionModeActivated = false;

	//temperatura rilevata
	double tempInput;
	//temperatura desiderata
	double tempSetpoint;
	//
	double tempOutput;

	//pin regolazione PWM
	int s1Pin;
	//pin regolazione temperatura
	int s2Pin;
	int s3Pin;
	int s4Pin;

	//aggressive and conservative PID Tuning Parameters
	double aggKp, aggKi, aggKd;
	double consKp, consKi, consKd;
	double pidMinValue = 0, pidMaxValue = 0;

	unsigned long windowStartTime;

	void InitializePid();
	void SetPidTuning();

public:
	FelfilControler();

	void SetupPwmOutputPin(int s1Pin);
	void SetupTemperatureOutputPins(int s2Pin, int s3Pin, int s4Pin);

	void SetupPidAggressiveTuning(double kp, double ki, double kd);
	void SetupPidConservativeTuning(double kp, double ki, double kd);
	void SetupPidOuputLimits(double min, double max);
	void Initialize();
	void Reset();

	void ControlPwm(int engineCurrent, int pwmSetpoint);
	TemperatureControlState ControlTemperature(double tempInput, double tempSetpoint);

	bool IsProtectionModeActivated();
};

#endif

