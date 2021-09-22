#include "FelfilController.h"

#define TimePassed 60000 //60 secondi
#define GapGoal 5

uint8_t temp_start = 0;
uint8_t temp_end   = 0;
bool delay_check = false;

double FelfilControler::getDifference(double first_value, double second_value)
{
  return second_value-first_value;
}

FelfilControler::FelfilControler()
{
	windowStartTime = millis();
	protectionModeActivated = false;
}

void FelfilControler::setTimerStart()
{
  timer_start = millis();
}

long int FelfilControler::getTimerStart()
{
  return timer_start;
}

void FelfilControler::setTimerEnd()
{
  timer_end = millis();
}

long int FelfilControler::getTimerEnd()
{
  return timer_end;
}

bool FelfilControler::hasTotTimePassed(long int _time_passed)
{
  bool flag = false;
  long int _timer_end = getTimerEnd();
  long int time_passed = timer_end - timer_start;
//  stampa("\ntempo passato: "); stampa((String)time_passed);

  if (time_passed > _time_passed)
    flag = true;

  return flag;
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
	double gap = (tempSetpoint - tempInput);
  if(gap <= -1) 
    StopHeating();
  else if(gap < 2)   //we're close to setpoint, use conservative tuning parameters
		pid->SetTunings(consKp, consKi, consKd);
	else if(gap >= 2) //we're far from setpoint, use aggressive tuning parameters
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

void FelfilControler::StopHeating()
{
  digitalWrite(s2Pin, LOW);
  digitalWrite(s3Pin, LOW);
  digitalWrite(s4Pin, LOW);
}

bool FelfilControler::isTempChecked()
{
  return temp_checked;
}

void FelfilControler::tempCheckedUpdate()
{
  temp_checked != temp_checked;
}

bool FelfilControler::isOnSetpoint(FelfilReader* felfilReader)
{
  return abs(tempSetpoint-felfilReader->ReadTemperature()) < 2;
}

void FelfilControler::checkTempError(FelfilReader* felfilReader, FelfilMenu* felfilMenu)
{
  setTimerEnd();
  felfilReader->setTempEnd();

  if(hasTotTimePassed(TimePassed))
  {
    setTimerStart();
    
    if(isTempChecked())
    {
      felfilReader->setTempStart();
      temp_start = felfilReader->getTempStart();
    }
    else
    {
      felfilReader->setTempEnd();
      temp_end = felfilReader->getTempEnd();
      
      double gap = temp_end-temp_start;
      if(gap<GapGoal)
      {
        if(felfilMenu->isHeating() && !isOnSetpoint(felfilReader))
          felfilMenu->GoToErrorMode(SENSORI);
      }
      else
      {
        felfilReader->setTempStart();
        temp_start = felfilReader->getTempStart();
      }
    }
    tempCheckedUpdate();
  }
}

TemperatureControlState FelfilControler::ControlTemperature(double tempInput, double tempSetpoint, FelfilReader* felfilReader, FelfilMenu* felfilMenu)
{
  this->tempInput = tempInput;
  this->tempSetpoint = tempSetpoint;

  SetPidTuning();
  
  //effettua il calcolo
  pid->Compute();

  //controlla che non ci sia troppa differenza tra window e millis
  if(!delay_check)
    if(millis() > 30000)
      windowStartTime = millis();
  
  //time to shift the Relay Window
  if (millis() - windowStartTime > pidMaxValue)
    windowStartTime += pidMaxValue;

  if ((tempOutput < millis() - windowStartTime) || (tempInput > tempSetpoint))
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
    
    checkTempError(felfilReader, felfilMenu);
    delay_check = true;

    if(tempInput-tempSetpoint > 20)
      felfilMenu->GoToErrorMode(HEAT_OVER_SETPOINT);
    
    return High;
  }
}

bool FelfilControler::IsProtectionModeActivated()
{
	return protectionModeActivated;
}

#pragma endregion
