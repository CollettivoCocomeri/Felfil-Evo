#include "FelfilMenu.h"

void FelfilMenuBackend::Initialize(cb_use menuUse, cb_change menuChange){
  menu = new MenuBackend(menuUse, menuChange);

  tempInit.addRight(tempWait);
  tempWait.addRight(pwmInit);
  pwmInit.addRight(tempRoot);

  tempRoot.addRight(tempSet);

  pwmRoot.addRight(pwmSet);
  pwmRoot.addBefore(tempRoot);
  pwmRoot.addAfter(tempRoot);

  menu->getRoot().addRight(tempInit);
  menu->getRoot().addLeft(errorMode);

  errorMode.addLeft(pwmProtectionMode);
  errorMode.addBefore(heatingOverSetpointError);
  errorMode.addAfter(errorCoppiaMode);
  
  errorCoppiaMode.addRight(contattoCoppiaError);
  errorCoppiaMode.addLeft(termocoppiaError);
  errorCoppiaMode.addAfter(risingTempError);

  menu->moveRight();
}

const char* FelfilMenuBackend::GetCurrentMenuName() {
  return menu->getCurrent().getName();
}

FelfilMenuBackend::FelfilMenuBackend(cb_use menuUse, cb_change menuChange) {
  Initialize(menuUse, menuChange);
}

MenuBackend* FelfilMenuBackend::GetMenu() {
  return menu;
}
    
bool FelfilMenuBackend::IsTempInit() {
  return GetCurrentMenuName() == TEMP_INIT;
}
bool FelfilMenuBackend::IsTempWait() {
  return GetCurrentMenuName() == TEMP_WAIT;
}
bool FelfilMenuBackend::IsPwmInit() {
  return GetCurrentMenuName() == PWM_INIT;
}
bool FelfilMenuBackend::IsTempRoot() {
  return GetCurrentMenuName() == MENU_TEMP_ROOT;
}
bool FelfilMenuBackend::IsTempSet() {
  return GetCurrentMenuName() == MENU_TEMP_SET;
}
bool FelfilMenuBackend::IsPwmRoot() {
  return GetCurrentMenuName() == MENU_PWM_ROOT;
}
bool FelfilMenuBackend::IsPwmSet() {
  return GetCurrentMenuName() == MENU_PWM_SET;
}
bool FelfilMenuBackend::IsErrorMode() {
  return GetCurrentMenuName() == ERROR_MODE;
}
bool FelfilMenuBackend::IsPwmProtectionMode() {
  return GetCurrentMenuName() == MENU_PWM_PROTECTION_MODE;
}
bool FelfilMenuBackend::IsErrorCoppiaMode() {
  return GetCurrentMenuName() == ERROR_COPPIA_MODE;
}
bool FelfilMenuBackend::IsRisingTempError() {
  return GetCurrentMenuName() == RISING_TEMPERATURE_ERROR;
}
bool FelfilMenuBackend::IsTermocoppiaError() {
  return GetCurrentMenuName() == TERMOCOPPIA_ERROR;
}
bool FelfilMenuBackend::IsContattoCoppiaError() {
  return GetCurrentMenuName() == CONTATTO_COPPIA_ERROR;
}
bool FelfilMenuBackend::IsHeatingOverSetpointError() {
  return GetCurrentMenuName() == HEATING_OVER_SETPOINT_ERROR;
}

void FelfilMenuBackend::ResetBackend(){
  menu->toRoot();
  menu->moveRight ();
}

void FelfilMenuBackend::GoToEngineProtectionMode(){
  menu->toRoot();
  menu->moveLeft(); //to errorMode
  menu->moveLeft();
}

void FelfilMenuBackend::GoToRisingTempError(){ 
  menu->toRoot();
  menu->moveLeft(); //to errorMode
  menu->moveDown(); //to errorCoppiaMode
  menu->moveDown();
}

void FelfilMenuBackend::GoToTermocoppiaError(){ 
  menu->toRoot();
  menu->moveLeft(); //to errorMode
  menu->moveDown(); //to errorCoppiaMode
  menu->moveLeft();
}

void FelfilMenuBackend::GoToContattoCoppiaError(){ 
  menu->toRoot();
  menu->moveLeft(); //to errorMode
  menu->moveDown(); //to errorCoppiaMode
  menu->moveRight();
}

void FelfilMenuBackend::GoToHeatingOverSetpointError(){ 
  menu->toRoot();
  menu->moveLeft(); //to errorMode
  menu->moveUp(); 
}

void FelfilMenuBackend::SaveLastSelectedMenu(){
  lastSelectedMenu = &(menu->getCurrent());
}

bool FelfilMenuBackend::IsMenuChanged(){
  return lastSelectedMenu->getName() != menu->getCurrent().getName();
}

#pragma region Init

ClickEncoder* FelfilMenu::encoder = NULL;
FelfilMenuBackend* FelfilMenu::menu = NULL;
bool FelfilMenu::isTempInitialized = false;

#pragma endregion

void FelfilMenu::ShowErrorMessage(String error){
  if (!menu->IsMenuChanged())
    return;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Check error code");
  lcd.setCursor(0, 1);
  lcd.print(error);
}

void FelfilMenu::ShowProtectionModeMessage(){
  if (!menu->IsMenuChanged())
    return;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pwm Protecion!");
  lcd.setCursor(0, 1);
  lcd.print("Click to restart.");
}

#pragma region Menu

void FelfilMenu::MenuUsed(MenuUseEvent used){
  if (menu->IsTempInit() || menu->IsPwmInit() || menu->IsPwmRoot() || menu->IsTempRoot())
  {
    if (menu->IsTempInit())
      isTempInitialized = true;

    /*if (menu->IsPwmInit())
      isPwmInitialized = true;
    */

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

  if (menu->IsPwmProtectionMode() || menu->IsRisingTempError() || menu->IsTermocoppiaError() || menu->IsContattoCoppiaError())
  {
    menu->ResetBackend();
    return;
  }
}

void FelfilMenu::MenuChanged(MenuChangeEvent changed){
  Serial.print("MENU CHANGED FROM: "); Serial.println(changed.from.getName());
  Serial.print("MENU CHANGED TO: "); Serial.println(changed.to.getName());
}

#pragma endregion

#pragma region Setup

FelfilMenu::FelfilMenu(){
  menu = new FelfilMenuBackend(FelfilMenu::MenuUsed, FelfilMenu::MenuChanged);
}

void FelfilMenu::SetupPwm(uint8_t defaultValue, uint8_t minValue, uint8_t maxValue, uint8_t step){
  defaultPwm = defaultValue;
  minPwm = minValue;
  maxPwm = maxValue;
  pwmStep = step;
}

void FelfilMenu::SetupTemperature(float defaultValue, float minValue, float maxValue, float step){
  defaultTemp = defaultValue;
  minTemp = minValue;
  maxTemp = maxValue;
  tempStep = step;
}

void FelfilMenu::SetHeatingMode(bool heating){
  this->heating = heating;
}

bool FelfilMenu::isHeating(){
  return this->heating;
}

void FelfilMenu::SetupClickEncoder(uint8_t A, uint8_t B, uint8_t BTN){
  encoder = new ClickEncoder(A, B, BTN);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(this->TimerIsr);
}

void FelfilMenu::SetupLcdlcd(){
  lcd.begin(16, 2);

  int charBitmapSize = (sizeof(charBitmap) / sizeof(charBitmap[0]));
  for (int i = 0; i < charBitmapSize; i++)
    lcd.createChar(i, (uint8_t *)charBitmap[i]);
}

void FelfilMenu::SetupEngineCurrentRefreshInterval(uint16_t millis){
  engineCurrentRefreshMillis = millis;
}

void FelfilMenu::TimerIsr(){
  encoder->service();
}

void FelfilMenu::ResetSetpoint(){
  isTempInitialized = false;
  pwmSetpoint = defaultPwm;
  tempSetpoint = defaultTemp;
}

void FelfilMenu::Reset(){
  ResetSetpoint();
  menu->ResetBackend();
}

void FelfilMenu::GoToErrorMode(String error){
  ResetSetpoint();
  if(error == SENSORI)
    menu->GoToRisingTempError();
  if(error == TERMOCOPPIA)
    menu->GoToTermocoppiaError();
  if(error == CONTATTO_COPPIA)
    menu->GoToContattoCoppiaError();
  if(error == PWM_PROTECTION)
    menu->GoToEngineProtectionMode();
  if(error == HEAT_OVER_SETPOINT)
    menu->GoToHeatingOverSetpointError();
}

#pragma endregion

#pragma region lcd

void FelfilMenu::PadNumber(float value){
  if (value < 10)
    lcd.print(" ");
  if (value < 100)
    lcd.print(" ");
}

void FelfilMenu::PrintSpaces(int tot){
  for (int i = 0; i < tot; i++)
  {
    lcd.print(" ");
  }
}

void FelfilMenu::ShowWellcomeMessage(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Felfil Evo   ");
  lcd.setCursor(0, 1);
  lcd.print("Hammers97");//Translate
  delay(2000);
}

void FelfilMenu::RefreshTemperatureInitlcd(){
  lcd.setCursor(0, 0);
  lcd.print("Temperature:");//Translate
  PrintSpaces(4);

  lcd.setCursor(0, 1);
  PrintSpaces(7);
  PadNumber(tempSetpoint);
  lcd.print(tempSetpoint, 1);

  lcd.setCursor(12, 1);
  lcd.print(char(3));
  lcd.setCursor(13, 1);
  lcd.print("C");
  PrintSpaces(3);
}

void FelfilMenu::RefreshTemperatureWaitlcd(){
  lcd.setCursor(0, 0);
  lcd.print("  ");
  lcd.setCursor(2, 0);
  PadNumber(tempInput);
  lcd.print(tempInput, 1);

  lcd.setCursor(7, 0);
  lcd.print("/");

  lcd.setCursor(8, 0);
  PadNumber(tempSetpoint);
  lcd.print(tempSetpoint, 1);

  lcd.setCursor(13, 0);
  lcd.print(char(3));
  lcd.setCursor(14, 0);
  lcd.print("C");
  PrintSpaces(1);

  lcd.setCursor(0, 1);
  lcd.print("    Heating!    ");//Translate
}

void FelfilMenu::RefreshPwmInitlcd(){
  lcd.setCursor(0, 0);
  lcd.print("Set speed:");//Translate
  PrintSpaces(10);

  lcd.setCursor(0, 1);
  PrintSpaces(1);
  lcd.print("RPM:");//Translate?
  lcd.print(pwmSetpoint);
  PrintSpaces(11);

  /*for (int i = 0; i < maxPwm; i++)
  {
    int cursorX = i + 3;
    lcd.setCursor(cursorX, 1);
    lcd.print(char(pwmSetpoint > i ? 0 : 1));
  }*/
  //PrintSpaces(13 - maxPwm);
}

void FelfilMenu::RefreshTemperaturelcdRow(){
  lcd.setCursor(0, 0);
  if (menu->IsTempRoot() || (menu->IsTempSet() && IsBlinkHighEdge(BLINK_INTERVAL_MILLS)))
  {
    lcd.print(char(2));
  }
  else
  {
    lcd.print(" ");
  }
  lcd.print(" ");
 
  lcd.setCursor(2, 0);
  PadNumber(tempInput);
  lcd.print(tempInput, 2);

  lcd.setCursor(7, 0);
  lcd.print("/");

  lcd.setCursor(8, 0);
  PadNumber(tempSetpoint);
  lcd.print(tempSetpoint, 1);

  lcd.setCursor(13, 0);
  lcd.print(char(3));
  lcd.setCursor(14, 0);
  lcd.print("C");

 lcd.print(heating ? char(0) : char(1));
}

void FelfilMenu::RefreshPwdlcdRow(){
  lcd.setCursor(0, 1);
  if (menu->IsPwmRoot() || (menu->IsPwmSet() && IsBlinkHighEdge(BLINK_INTERVAL_MILLS)))
  {
    lcd.print(char(2));
  }
  else
  {
    lcd.print(" ");
  }

  lcd.setCursor(1, 1);
  PrintSpaces(1);
  lcd.print("RPM:");
  lcd.print(pwmSetpoint);

  lcd.print("   A:");
  lcd.print(engineCurrentInput, 2);

  PrintSpaces(6 - maxPwm);
}

void FelfilMenu::Refreshlcd(){
  if (menu->IsRisingTempError())
  {
    ShowErrorMessage(SENSORI);
    return;
  }
  
  if (menu->IsTermocoppiaError())
  {
    ShowErrorMessage(TERMOCOPPIA);
    return;
  }
  
  if (menu->IsContattoCoppiaError())
  {
    ShowErrorMessage(CONTATTO_COPPIA);
    return;
  }
  
  if (menu->IsPwmProtectionMode())
  {
    ShowProtectionModeMessage();
    return;
  }
  
  if (menu->IsHeatingOverSetpointError())
  {
    ShowErrorMessage(HEAT_OVER_SETPOINT);
    return;
  }
  
  if (menu->IsTempInit())
  {
    RefreshTemperatureInitlcd();
    return;
  }

  if (menu->IsTempWait())
  {
    RefreshTemperatureWaitlcd();
    return;
  }

  if (menu->IsPwmInit())
  {
    RefreshPwmInitlcd();
    return;
  }

  RefreshTemperaturelcdRow();
  RefreshPwdlcdRow();
}

bool FelfilMenu::IsBlinkHighEdge(uint8_t edgeMilliseconds){
  return (millis() / edgeMilliseconds) % 2 == 0;
}

#pragma endregion

#pragma region Data Access

int FelfilMenu::GetPwmSetpoint(){
  return pwmSetpoint;
}

double FelfilMenu::GetTempSetpoint(){
  return tempSetpoint;
}

void FelfilMenu::TemperatureReached(){
  if (menu->IsTempWait())
    menu->GetMenu()->moveRight();
}

bool FelfilMenu::IsTempSetpointInitialized(){
  return isTempInitialized;
}

#pragma endregion

#pragma region Read Input

void FelfilMenu::ReadEncoder(){
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
  
  if (menu->IsRisingTempError() || menu->IsTermocoppiaError() || menu->IsContattoCoppiaError() || menu->IsHeatingOverSetpointError())
  {
    // ReadMenuEncoderClick();
    return;
  }

  if (menu->IsPwmProtectionMode())
  {
    //ReadMenuEncoderClick();
    ReadEncoderInPwmSafeMode();
    return;
  }

  ReadMenuEncoderMovement();
  ReadMenuEncoderClick();
}

void FelfilMenu::ReadEncoderInPwmSafeMode(){
  int16_t encoderValue = encoder->getValue();
  ClickEncoder::Button encoderButton = encoder->getButton();

  if (encoderButton == ClickEncoder::Clicked ||
      encoderButton == ClickEncoder::DoubleClicked ||
      encoderButton == ClickEncoder::Held)
    menu->GetMenu()->use();
}

void FelfilMenu::ReadMenuEncoderMovement(){
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

void FelfilMenu::ReadMenuEncoderClick(){
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

void FelfilMenu::ReadPwm(){
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

void FelfilMenu::SetPwm(int value){
  pwmSetpoint = value;
}

void FelfilMenu::ReadTemperature(){
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

void FelfilMenu::SetTempInput(double tempInput){
  this->tempInput = tempInput;
}

void FelfilMenu::SetEngineCurrentInput(float engineCurrentInput){
  unsigned long now = millis();
  if (now - lastEngineCurrentSampleTime < engineCurrentRefreshMillis)
    return;

  this->engineCurrentInput = engineCurrentInput;
  lastEngineCurrentSampleTime = now;
}

void FelfilMenu::Loop(){
  //resetta i valori di default alla prima iterazione
  if (isFirstLoop)
  {
    ShowWellcomeMessage();
    ResetSetpoint();
    isFirstLoop = false;
  }

  Refreshlcd();

  menu->SaveLastSelectedMenu();
  ReadEncoder();
}

#pragma endregion
