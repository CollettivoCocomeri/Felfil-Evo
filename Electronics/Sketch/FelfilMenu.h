// FelfilMenu.h

#ifndef _FELFILMENU_h
#define _FELFILMENU_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#pragma once
#include <MenuBackend.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
//#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <WString.h>

#define TEMP_INIT "TempInit"
#define TEMP_WAIT "TempWait"
#define PWM_INIT "PwmInit"
#define MENU_TEMP_ROOT "TempRoot"
#define MENU_TEMP_SET "TempSet"
#define MENU_PWM_ROOT "PwmRoot"
#define MENU_PWM_SET "PwmSet"
#define MENU_PWM_PROTECTION_MODE "PwmProtectionMode"

#define BLINK_INTERVAL_MILLS 2000
#define DEFAULT_ENGINE_CURRENT_REFRESH_MILLIS 500

//LCD Char
const uint8_t charBitmap[][8] = {
	{ 0x4, 0xa, 0xa, 0x11, 0x15, 0x15, 0xe, 0 },//quadratino vuoto 0
	{ 0, 0, 0, 0, 0, 0, 0, 0 },//quadratino pieno 1
	{ 0, 0x8, 0xc, 0x1e, 0x1f, 0x1e, 0xc, 0x8 },//freccina 2
	{ 0xc, 0x12, 0x12, 0xc, 0, 0, 0, 0 } };//gradi;

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
	MenuItem pwmProtectionMode = MenuItem(MENU_PWM_PROTECTION_MODE);
	MenuItem* lastSelectedMenu;

	void Initialize(cb_use menuUse, cb_change menuChange)
	{
		menu = new MenuBackend(menuUse, menuChange);

		tempInit.addRight(tempWait);
		tempWait.addRight(pwmInit);
		pwmInit.addRight(tempRoot);

		tempRoot.addRight(tempSet);

		pwmRoot.addRight(pwmSet);
		pwmRoot.addBefore(tempRoot);
		pwmRoot.addAfter(tempRoot);

		menu->getRoot().addRight(tempInit);
		menu->getRoot().addLeft(pwmProtectionMode);
		menu->moveRight();
	};
	const char* GetCurrentMenuName() { return menu->getCurrent().getName(); };

public:
	FelfilMenuBackend(cb_use menuUse, cb_change menuChange) { Initialize(menuUse, menuChange); };
	MenuBackend* GetMenu() { return menu; };
	bool IsMenuRoot() { return GetCurrentMenuName() == "MenuRoot"; }
	bool IsTempInit() { return GetCurrentMenuName() == TEMP_INIT; }
	bool IsTempWait() { return GetCurrentMenuName() == TEMP_WAIT; }
	bool IsPwmInit() { return GetCurrentMenuName() == PWM_INIT; }
	bool IsTempRoot() { return GetCurrentMenuName() == MENU_TEMP_ROOT; }
	bool IsTempSet() { return GetCurrentMenuName() == MENU_TEMP_SET; }
	bool IsPwmRoot() { return GetCurrentMenuName() == MENU_PWM_ROOT; }
	bool IsPwmSet() { return GetCurrentMenuName() == MENU_PWM_SET; }
	bool IsPwmProtectionMode() { return GetCurrentMenuName() == MENU_PWM_PROTECTION_MODE; }

	void Reset() 
	{ 
		menu->toRoot(); 
		menu->moveRight();
	}

	void GoToEngineProtectionMode()
	{
		menu->toRoot();
		menu->moveLeft();
	}

	void SaveLastSelectedMenu()
	{
		lastSelectedMenu = &(menu->getCurrent());
	}

	bool IsMenuChanged()
	{
		return lastSelectedMenu->getName() != menu->getCurrent().getName();
	}
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
	float tempInput;
	static bool isTempInitialized;

	float engineCurrentInput;

	static FelfilMenuBackend* menu;

	static void MenuUsed(MenuUseEvent used);
	static void MenuChanged(MenuChangeEvent changed);

	LCD *display;
	uint16_t engineCurrentRefreshMillis = DEFAULT_ENGINE_CURRENT_REFRESH_MILLIS;
	unsigned long lastEngineCurrentSampleTime = 0;

	ClickEncoder::Button lastClickedButton;
	static ClickEncoder *encoder;
	static void TimerIsr();

	void PadNumber(float value);
	void PrintSpaces(int tot);

	void ShowWellcomeMessage();
	void ShowProtectionModeMessage();
	void RefreshTemperatureInitDisplay();
	void RefreshTemperatureWaitDisplay();
	void RefreshTemperatureDisplayRow();
	void RefreshDisplay();
	void RefreshPwmInitDisplay();
	void RefreshPwdDisplayRow();

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
	void SetupLcdDisplay(uint8_t lcd_Addr, uint8_t En, uint8_t Rw, uint8_t Rs, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlighPin, t_backlighPol pol);
	void SetupEngineCurrentRefreshInterval(uint16_t millis);

	void SetTempInput(int tempInput);
	void SetEngineCurrentInput(float currentInput);

	bool IsTempSetpointInitialized();

	void Loop();
	void ResetSetpoint();
	void Reset();

	void GoToProtectionMode();

	int GetPwmSetpoint();
	double GetTempSetpoint();

	void TemperatureReached();
};

#pragma region Init

ClickEncoder* FelfilMenu::encoder = NULL;
FelfilMenuBackend* FelfilMenu::menu = NULL;
bool FelfilMenu::isTempInitialized = false;

#pragma endregion

#pragma region Menu

void FelfilMenu::MenuUsed(MenuUseEvent used)
{
	if (menu->IsTempInit() || menu->IsPwmInit() || menu->IsPwmRoot() || menu->IsTempRoot())
	{
		if (menu->IsTempInit())
		{
			isTempInitialized = true;
		}

		/*if (menu->IsPwmInit())
		{
			isPwmInitialized = true;
		}*/

		menu->GetMenu()->moveRight();
		return;
	}

	if (menu->IsPwmSet())
	{
		menu->GetMenu()->moveLeft();
		return;
	}

	if (menu->IsTempSet())
	{
		menu->GetMenu()->moveLeft();
		return;
	}

	if (menu->IsPwmProtectionMode())
	{
		menu->Reset();
		return;
	}
}

void FelfilMenu::MenuChanged(MenuChangeEvent changed)
{
	Serial.print("MENU CHANGED FROM: "); Serial.println(changed.from.getName());
	Serial.print("MENU CHANGED TO: "); Serial.println(changed.to.getName());
}

#pragma endregion

#pragma region Setup

FelfilMenu::FelfilMenu()
{
	menu = new FelfilMenuBackend(FelfilMenu::MenuUsed, FelfilMenu::MenuChanged);
}

void FelfilMenu::SetupPwm(uint8_t defaultValue, uint8_t minValue, uint8_t maxValue, uint8_t step)
{
	defaultPwm = defaultValue;
	minPwm = minValue;
	maxPwm = maxValue;
	pwmStep = step;
}

void FelfilMenu::SetupTemperature(float defaultValue, float minValue, float maxValue, float step)
{
	defaultTemp = defaultValue;
	minTemp = minValue;
	maxTemp = maxValue;
	tempStep = step;
}

void FelfilMenu::SetHeatingMode(bool heating)
{
	this->heating = heating;
}

void FelfilMenu::SetupClickEncoder(uint8_t A, uint8_t B, uint8_t BTN)
{
	encoder = new ClickEncoder(A, B, BTN);

	Timer1.initialize(1000);
	Timer1.attachInterrupt(this->TimerIsr);
}

void FelfilMenu::SetupLcdDisplay(uint8_t lcd_Addr, uint8_t En, uint8_t Rw, uint8_t Rs, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlighPin, t_backlighPol pol)
{
	display = new LiquidCrystal_I2C(lcd_Addr, En, Rw, Rs, d4, d5, d6, d7, backlighPin, pol);
	display->begin(16, 2);

	int charBitmapSize = (sizeof(charBitmap) / sizeof(charBitmap[0]));
	for (int i = 0; i < charBitmapSize; i++)
	{
		display->createChar(i, (uint8_t *)charBitmap[i]);
	}
}

void FelfilMenu::SetupEngineCurrentRefreshInterval(uint16_t millis)
{
	engineCurrentRefreshMillis = millis;
}

void FelfilMenu::TimerIsr()
{
	encoder->service();
}

void FelfilMenu::ResetSetpoint()
{
	isTempInitialized = false;
	pwmSetpoint = defaultPwm;
	tempSetpoint = defaultTemp;
}

void FelfilMenu::Reset()
{
	ResetSetpoint();
	menu->Reset();
}

void FelfilMenu::GoToProtectionMode()
{
	ResetSetpoint();
	menu->GoToEngineProtectionMode();
}

#pragma endregion

#pragma region Display

void FelfilMenu::PadNumber(float value)
{
	if (value < 10)
		display->print(" ");
	if (value < 100)
		display->print(" ");
}

void FelfilMenu::PrintSpaces(int tot)
{
	for (int i = 0; i < tot; i++)
	{
		display->print(" ");
	}
}

void FelfilMenu::ShowWellcomeMessage()
{
	display->clear();
	display->setCursor(0, 0);
	display->print("   Felfil Evo   ");
	display->setCursor(0, 1);
	display->print("firmware Ver.0.8");
	delay(2000);
}

void FelfilMenu::ShowProtectionModeMessage()
{
	if (!menu->IsMenuChanged())
		return;

	display->clear();
	display->setCursor(0, 0);
	display->print("Protection Mode!");
	display->setCursor(0, 1);
	display->print("Setting resetted");
}

void FelfilMenu::RefreshTemperatureInitDisplay()
{
	display->setCursor(0, 0);
	display->print("Temperature:");
	PrintSpaces(4);

	display->setCursor(0, 1);
	PrintSpaces(7);
	PadNumber(tempSetpoint);
	display->print(tempSetpoint, 1);

	display->setCursor(12, 1);
	display->print(char(3));
	display->setCursor(13, 1);
	display->print("C");
	PrintSpaces(3);
}

void FelfilMenu::RefreshTemperatureWaitDisplay()
{
	display->setCursor(0, 0);
	display->print("  ");
	display->setCursor(2, 0);
	PadNumber(tempInput);
	display->print(tempInput, 2);

	display->setCursor(7, 0);
	display->print("/");

	display->setCursor(8, 0);
	PadNumber(tempSetpoint);
	display->print(tempSetpoint, 1);

	display->setCursor(13, 0);
	display->print(char(3));
	display->setCursor(14, 0);
	display->print("C");
	PrintSpaces(1);

	display->setCursor(0, 1);
	display->print("    Heating!    ");
}

void FelfilMenu::RefreshPwmInitDisplay()
{
	display->setCursor(0, 0);
	display->print("Set speed:");
	PrintSpaces(10);

	display->setCursor(0, 1);
	PrintSpaces(1);
	display->print("RPM:");
	display->print(pwmSetpoint);
	PrintSpaces(11);

	/*for (int i = 0; i < maxPwm; i++)
	{
		int cursorX = i + 3;
		display->setCursor(cursorX, 1);
		display->print(char(pwmSetpoint > i ? 0 : 1));
	}*/
	//PrintSpaces(13 - maxPwm);
}

void FelfilMenu::RefreshTemperatureDisplayRow()
{
	display->setCursor(0, 0);
	if (menu->IsTempRoot() || (menu->IsTempSet() && IsBlinkHighEdge(BLINK_INTERVAL_MILLS)))
	{
		display->print(char(2));
	}
	else
	{
		display->print(" ");
	}
	display->print(" ");

	display->setCursor(2, 0);
	PadNumber(tempInput);
	display->print(tempInput, 2);

	display->setCursor(7, 0);
	display->print("/");

	display->setCursor(8, 0);
	PadNumber(tempSetpoint);
	display->print(tempSetpoint, 1);

	display->setCursor(13, 0);
	display->print(char(3));
	display->setCursor(14, 0);
	display->print("C");
	
	display->print(heating ? char(0) : char(1));
}

void FelfilMenu::RefreshPwdDisplayRow() 
{
	display->setCursor(0, 1);
	if (menu->IsPwmRoot() || (menu->IsPwmSet() && IsBlinkHighEdge(BLINK_INTERVAL_MILLS)))
	{
		display->print(char(2));
	}
	else
	{
		display->print(" ");
	}

	//display->setCursor(1, 1);
	//display->print("M:");

	//for (int i = 0; i < maxPwm; i++)
	//{
	//	int cursorX = i + 3;
	//	display->setCursor(cursorX, 1);
	//	display->print(char(pwmSetpoint > i ? 0 : 1));
	//}

	display->setCursor(1, 1);
	PrintSpaces(1);
	display->print("RPM:");
	display->print(pwmSetpoint);

	display->print("   A:");
	display->print(engineCurrentInput, 2);

	PrintSpaces(6 - maxPwm);
}

void FelfilMenu::RefreshDisplay()
{
	if (menu->IsTempInit())
	{
		RefreshTemperatureInitDisplay();
		return;
	}

	if (menu->IsTempWait())
	{
		RefreshTemperatureWaitDisplay();
		return;
	}

	if (menu->IsPwmInit())
	{
		RefreshPwmInitDisplay();
		return;
	}

	if (menu->IsPwmProtectionMode())
	{
		ShowProtectionModeMessage();
		return;
	}
	
	RefreshTemperatureDisplayRow();
	RefreshPwdDisplayRow();
}

bool FelfilMenu::IsBlinkHighEdge(uint8_t edgeMilliseconds)
{
	return (millis() / edgeMilliseconds) % 2 == 0;
}

#pragma endregion

#pragma region Data Access

int FelfilMenu::GetPwmSetpoint()
{
	return pwmSetpoint;
}

double FelfilMenu::GetTempSetpoint()
{
	return tempSetpoint;
}

void FelfilMenu::TemperatureReached()
{
	if (menu->IsTempWait())
		menu->GetMenu()->moveRight();
}

bool FelfilMenu::IsTempSetpointInitialized()
{
	return isTempInitialized;
}

#pragma endregion

#pragma region Read Input

void FelfilMenu::ReadEncoder()
{
	if (menu->IsPwmInit() || menu->IsPwmSet())
	{
		ReadPwm();
		ReadMenuEncoderClick();
		return;
	}

	if (menu->IsTempInit() || menu->IsTempSet())
	{
		ReadTemperature();
		ReadMenuEncoderClick();
		return;
	}

	if (menu->IsPwmProtectionMode())
	{
		ReadEncoderInPwmSafeMode();
		return;
	}

	ReadMenuEncoderMovement();
	ReadMenuEncoderClick();
}

void FelfilMenu::ReadEncoderInPwmSafeMode()
{
	int16_t encoderValue = encoder->getValue();
	ClickEncoder::Button encoderButton = encoder->getButton();

	if (encoderValue != 0 || 
		encoderButton == ClickEncoder::Clicked || 
		encoderButton == ClickEncoder::DoubleClicked || 
		encoderButton == ClickEncoder::Held)
		menu->GetMenu()->use();
}

void FelfilMenu::ReadMenuEncoderMovement()
{
	int16_t encoderValue = encoder->getValue();
	if (encoderValue < 0)
	{
		menu->GetMenu()->moveUp();
	}
	else if (encoderValue > 0)
	{
		menu->GetMenu()->moveDown();
	}
}

void FelfilMenu::ReadMenuEncoderClick()
{
	ClickEncoder::Button encoderButton = encoder->getButton();
	
	switch (encoderButton)
	{
	case ClickEncoder::DoubleClicked:
		SetPwm(0);
		break;
	case ClickEncoder::Clicked:
		menu->GetMenu()->use();
		break;
	case ClickEncoder::Held:
		if (lastClickedButton != ClickEncoder::Held)
			Reset();
		break;
	}

	lastClickedButton = encoderButton;
}

void FelfilMenu::ReadPwm()
{
	int16_t encoderValue = encoder->getValue();
	int16_t newPwmSetpoint = pwmSetpoint + (encoderValue * pwmStep);
	if (newPwmSetpoint > maxPwm)
	{
		newPwmSetpoint = maxPwm;
	}
	else if (newPwmSetpoint < minPwm)
	{
		newPwmSetpoint = minPwm;
	}
	pwmSetpoint = newPwmSetpoint;
}

void FelfilMenu::SetPwm(int value)
{
	pwmSetpoint = value;
}

void FelfilMenu::ReadTemperature()
{
	int16_t encoderValue = encoder->getValue();
	float newTempSetpoint = tempSetpoint + encoderValue * tempStep;
	if (newTempSetpoint > maxTemp)
	{
		newTempSetpoint = maxTemp;
	}
	else if (newTempSetpoint < minTemp)
	{
		newTempSetpoint = minTemp;
	}
	tempSetpoint = newTempSetpoint;
}

#pragma endregion

#pragma region Loop

void FelfilMenu::SetTempInput(int tempInput)
{
	this->tempInput = tempInput;
}

void FelfilMenu::SetEngineCurrentInput(float engineCurrentInput)
{
	unsigned long now = millis();
	if (now - lastEngineCurrentSampleTime < engineCurrentRefreshMillis)
		return;

	this->engineCurrentInput = engineCurrentInput;
	lastEngineCurrentSampleTime = now;
}

void FelfilMenu::Loop()
{
	//resetta i valori di default alla prima iterazione
	if (isFirstLoop)
	{
		ShowWellcomeMessage();
		ResetSetpoint();
		isFirstLoop = false;
	}

	RefreshDisplay();
	
	menu->SaveLastSelectedMenu();
	ReadEncoder();
}

#pragma endregion

#endif

