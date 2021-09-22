#ifndef _FELFILMENU_h
#define _FELFILMENU_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#pragma once
#include <MenuBackend_copia.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

#define TEMP_INIT "TempInit"
#define TEMP_WAIT "TempWait"
#define PWM_INIT "PwmInit"
#define MENU_TEMP_ROOT "TempRoot"
#define MENU_TEMP_SET "TempSet"
#define MENU_PWM_ROOT "PwmRoot"
#define MENU_PWM_SET "PwmSet"
#define ERROR_MODE "ErrorMode"
#define ERROR_COPPIA_MODE "ErrorCoppiaMode"
#define MENU_PWM_PROTECTION_MODE "PwmProtectionMode"
#define RISING_TEMPERATURE_ERROR "RisingTemperatureError"
#define TERMOCOPPIA_ERROR "TermocoppiaError"
#define CONTATTO_COPPIA_ERROR "ContattoTermocoppiaError"
#define HEATING_OVER_SETPOINT_ERROR "HeatingOverSetpointError"

//Errori
#define TERMOCOPPIA         "TERMOCOPPIA" // 0-750 (rotto oppure coppia al contrario)
#define SENSORI             "SENSORI_TEMP" // temperatura non sale quando scalda
#define CONTATTO_COPPIA     "CONTATTO_COPPIA" // c'è un intervallo troppo elevato tra un valore letto e il precedenze
#define PWM_PROTECTION      "PWM_PROTECTION" // protection mode (assirbimento motore > 2)
#define HEAT_OVER_SETPOINT  "HEAT_IN_SETPOINT" // scalda nonostante il raggiungimento del setpoint 

#define BLINK_INTERVAL_MILLS 2000
#define DEFAULT_ENGINE_CURRENT_REFRESH_MILLIS 500

//LCD 
const uint8_t charBitmap[][8] = {
  { 0x4, 0xa, 0xa, 0x11, 0x15, 0x15, 0xe, 0 },//fiammella 
  { 0, 0, 0, 0, 0, 0, 0, 0 },//quadratino pieno 1
  { 0, 0x8, 0xc, 0x1e, 0x1f, 0x1e, 0xc, 0x8 },//freccina 2
  { 0xc, 0x12, 0x12, 0xc, 0, 0, 0, 0 }
};//gradi;

class FelfilMenuBackend
{
  private:

    MenuBackend* menu;
    MenuItem tempInit = MenuItem(TEMP_INIT);
    MenuItem tempWait = MenuItem(TEMP_WAIT);
    MenuItem tempRoot = MenuItem(MENU_TEMP_ROOT);
    MenuItem tempSet = MenuItem(MENU_TEMP_SET);
    MenuItem pwmInit = MenuItem(PWM_INIT);
    MenuItem pwmRoot = MenuItem(MENU_PWM_ROOT);
    MenuItem pwmSet = MenuItem(MENU_PWM_SET);
    MenuItem errorMode = MenuItem(ERROR_MODE);
    MenuItem errorCoppiaMode = MenuItem(ERROR_COPPIA_MODE);
    MenuItem pwmProtectionMode = MenuItem(MENU_PWM_PROTECTION_MODE);
    MenuItem risingTempError = MenuItem(RISING_TEMPERATURE_ERROR);
    MenuItem termocoppiaError = MenuItem(TERMOCOPPIA_ERROR);
    MenuItem contattoCoppiaError = MenuItem(CONTATTO_COPPIA_ERROR);
    MenuItem heatingOverSetpointError = MenuItem(HEATING_OVER_SETPOINT_ERROR);
    MenuItem* lastSelectedMenu;

    void Initialize(cb_use menuUse, cb_change menuChange);
    
    const char* GetCurrentMenuName();

  public:
    FelfilMenuBackend(cb_use menuUse, cb_change menuChange);
    
    MenuBackend* GetMenu();
    
    bool IsTempInit();
    bool IsTempWait();
    bool IsPwmInit();
    bool IsTempRoot();
    bool IsTempSet();
    bool IsPwmRoot();
    bool IsPwmSet();
    bool IsErrorMode();
    bool IsErrorCoppiaMode();
    bool IsPwmProtectionMode();
    bool IsRisingTempError();
    bool IsTermocoppiaError();
    bool IsContattoCoppiaError();
    bool IsHeatingOverSetpointError();

    void ResetBackend();

    void GoToEngineProtectionMode();
    void GoToRisingTempError();
    void GoToTermocoppiaError();
    void GoToContattoCoppiaError();
    void GoToHeatingOverSetpointError();

    void SaveLastSelectedMenu();

    bool IsMenuChanged();
};

class FelfilMenu
{
  private:

    uint8_t defaultPwm;
    uint8_t minPwm;
    uint8_t maxPwm;
    uint8_t pwmStep;
    uint8_t pwmSetpoint;

    bool heating = false;

    float defaultTemp;
    float minTemp;
    float maxTemp;
    float tempStep;
    float tempSetpoint;
    double tempInput;
    float tempOutput;
    static bool isTempInitialized;
    
    float engineCurrentInput;

    static FelfilMenuBackend* menu;

    static void MenuUsed(MenuUseEvent used);
    static void MenuChanged(MenuChangeEvent changed);

    hd44780_I2Cexp lcd;
    uint16_t engineCurrentRefreshMillis = DEFAULT_ENGINE_CURRENT_REFRESH_MILLIS;
    unsigned long lastEngineCurrentSampleTime = 0;

    ClickEncoder::Button lastClickedButton;
    static ClickEncoder *encoder;
    static void TimerIsr();

    void PadNumber(float value);
    void PrintSpaces(int tot);

    void ShowWellcomeMessage();
    void ShowProtectionModeMessage();
    void RefreshTemperatureInitlcd();
    void RefreshTemperatureWaitlcd();
    void RefreshTemperaturelcdRow();
    void Refreshlcd();
    void RefreshPwmInitlcd();
    void RefreshPwdlcdRow();

    void ReadEncoder();
    void ReadMenuEncoderMovement();
    void ReadMenuEncoderClick();
    void ReadEncoderInPwmSafeMode();
    void ReadPwm();
    void SetPwm(int value);
    void ReadTemperature();

    bool isFirstLoop = true;
    bool IsBlinkHighEdge(uint8_t edgeMilliseconds);

  public:

    FelfilMenu();

    void SetupPwm(uint8_t defaultValue, uint8_t minValue, uint8_t maxValue, uint8_t step);
    void SetupTemperature(float defaultValue, float minValue, float maxValue, float step);
    void SetHeatingMode(bool heating);

    void SetupClickEncoder(uint8_t A, uint8_t B, uint8_t BTN);
    void SetupLcdlcd();
    void SetupEngineCurrentRefreshInterval(uint16_t millis);

    void SetTempInput(double tempInput);
    void SetEngineCurrentInput(float currentInput);

    bool IsTempSetpointInitialized();
    
    bool isHeating();
    
    void Loop();
    void ResetSetpoint();
    void Reset();

    void GoToProtectionMode();

    void GoToErrorMode(String error);

    int GetPwmSetpoint();
    double GetTempSetpoint();

    void TemperatureReached();
    
    void ShowErrorMessage(String error);
};

#endif
