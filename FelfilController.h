// FelfilController.h
#ifndef _FELFILCONTROLLER_h
#define _FELFILCONTROLLER_h

#include <FelfilPID.h>
//#include "FakePid.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "FelfilReader.h"
#include "FelfilMenu.h"

enum TemperatureControlState
{
	Low = 0,
	High = 1
};

//#error The class declaration for Creature is indeed being compiled
class FelfilControler
{
private:
	PID* pid;
	
	//attivazione modalità di protezione
	bool protectionModeActivated = false;

  //ci dice se sta scaldando
  bool heatingState = false;

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
  long int timer_start = 0;
  long int timer_end   = 0;
  bool temp_checked    = false;
	
  FelfilControler();
  
	void SetupPwmOutputPin(int s1Pin);
	void SetupTemperatureOutputPins(int s2Pin, int s3Pin, int s4Pin);

	void SetupPidAggressiveTuning(double kp, double ki, double kd);
  void SetupPidConservativeTuning(double kp, double ki, double kd);
	void SetupPidOuputLimits(double min, double max);
	void Initialize();
	void Reset();

	void ControlPwm(int engineCurrent, int pwmSetpoint);
	void StopEngine();
  void StopHeating();
	TemperatureControlState ControlTemperature(double tempInput, double tempSetpoint, FelfilReader* felfilReader, FelfilMenu* felfilMenu);

	bool IsProtectionModeActivated();

  double getDifference(double first_value, double second_value);
  
  void setTimerStart();
  void setTimerEnd();
  long int getTimerStart();
  long int getTimerEnd();
  bool isTempChecked();
  void tempCheckedUpdate();
  bool hasTotTimePassed(long int _time_passed);
  bool isOnSetpoint(FelfilReader* felfilReader);

  void checkTempError(FelfilReader* felfilReader, FelfilMenu* felfilMenu);
};

#endif
