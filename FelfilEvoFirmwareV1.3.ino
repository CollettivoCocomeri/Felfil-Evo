/* 05/04/2017 Felfil Evo Firmware Version 0.9.1  International */

/* For who wants to translate the Firmware language, just search for "Translate" comment, you will find all the text lcded in the firmware  */

#include "FelfilReader.h"
#include "FelfilController.h"
#include "FelfilMenu.h"
#pragma once
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <WString.h>
#include <Max6675.h>

#define Ts_Pin_Dt 12
#define Ts_Pin_Ss 10
#define Ts_Pin_Clk 13
#define Ts_Offset 0 // offset for temperature measurement.1 stannds for 0.25 Celsius

#define PwmPin 3
#define TempPin1 4
#define TempPin2 5
#define TempPin3 6

#define PWM_InputCurrentPin A0

#define EngineCurrentSamples 50
#define EngineCurrentSampleTime 1
#define EngineCurrentOffset -0.2

#define TemperatureSamples 5
#define TemperatureSampleTime 300

#define EngineCurrentLcdRefreshInterval 500

#define TemperatureReachedOffset 3

#define GapOldTemp 53

double old_temperature = 1.0;
bool flag = false;
float time_init = 0.0;

Max6675 ts(Ts_Pin_Dt, Ts_Pin_Ss, Ts_Pin_Clk, Ts_Offset);

FelfilMenu* felfilMenu;
FelfilControler* felfilController;
FelfilReader* felfilReader;

TemperatureControlState temperatureControlState = Low;

// the setup function runs once when you press reset or power the board
void setup() {
  //setup menu
  felfilMenu = new FelfilMenu();
  felfilMenu->SetupPwm(0, 0, 9, 1);
  felfilMenu->SetupTemperature(150, 25, 250, 1);
  felfilMenu->SetupLcdlcd();
  felfilMenu->SetupClickEncoder(9, 8, 7);
  felfilMenu->SetupEngineCurrentRefreshInterval(EngineCurrentLcdRefreshInterval);

  //setup controller
  felfilController = new FelfilControler();
  felfilController->SetupPidOuputLimits(0, 255);
  felfilController->SetupPidAggressiveTuning(255, 0, 0);
  felfilController->SetupPidConservativeTuning(63.53, 2.48, 407.22);//407.22
  felfilController->SetupPwmOutputPin(PwmPin);
  felfilController->SetupTemperatureOutputPins(TempPin1, TempPin2, TempPin3);
  felfilController->Initialize();
  
  delay(1000);
  //setup reader
  felfilReader = new FelfilReader(&ts, PWM_InputCurrentPin);
  felfilReader->SetupEngineCurrentSamples(EngineCurrentSamples, EngineCurrentSampleTime);
  felfilReader->SetupTemperatureSamples(TemperatureSamples, TemperatureSampleTime);
  // TENERE DA CONTO!!!!!
  digitalWrite(TempPin1, LOW);
  digitalWrite(TempPin2, LOW);
  digitalWrite(TempPin3, LOW);
  TemperatureControlState (LOW);
  // TENERE DA CONTO!!!!! -end

  felfilController->setTimerStart();
  felfilController->setTimerEnd();
  felfilReader->setTempStart();
  felfilReader->setTempEnd();
}

// the loop function runs over and over again until power down or reset
void loop() {
  //rilevazione temperatura e corrente sul motore
  double temperature = felfilReader->ReadTemperature();
  float engineCurrent = felfilReader->ReadEngineCurrent(temperatureControlState == High ? EngineCurrentOffset : 0);

  //gestione contatto termocoppia
  if(temperature>0 && !flag)
  {
    old_temperature = temperature;
    flag = true;
  }
  else if(flag)
    if(abs(temperature-old_temperature) > GapOldTemp)
      felfilMenu->GoToErrorMode(CONTATTO_COPPIA);
    else
      old_temperature = temperature;
  
  //gestione errore termocoppia
  if(millis()-time_init>10000)
    if(temperature == 0.0 || temperature == 750.0)
      felfilMenu->GoToErrorMode(TERMOCOPPIA);

  //gestione lcd
  felfilMenu->SetTempInput(temperature);
  felfilMenu->SetEngineCurrentInput(engineCurrent);
  felfilMenu->Loop();

  //setpoint iniziali non settati
  if (!felfilMenu->IsTempSetpointInitialized())
  {
    felfilController->StopEngine();
    felfilController->StopHeating();
    return;
  }
  
  //lettura valori di setpoint
  int pwmSetpoint = felfilMenu->GetPwmSetpoint();
  double tempSetpoint = felfilMenu->GetTempSetpoint();

  //gestione attuatore temperatura
  temperatureControlState = felfilController->ControlTemperature(temperature, tempSetpoint, felfilReader, felfilMenu);
  felfilMenu->SetHeatingMode(temperatureControlState == High);
 
  //controllo se sono in temperatura
  if (temperature >= tempSetpoint - TemperatureReachedOffset)
    felfilMenu->TemperatureReached();

  //gestione attuatore motore
  felfilController->ControlPwm(engineCurrent, pwmSetpoint);

  //controllo di sicurezza per eventuale reset
  if (felfilController->IsProtectionModeActivated())
  {
    felfilController->Reset();
    felfilMenu->GoToErrorMode(PWM_PROTECTION);
  }
}
