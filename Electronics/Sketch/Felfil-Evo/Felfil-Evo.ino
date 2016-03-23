#include <MenuBackend.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <PID_v1.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include "ThermistorConverter.h"

//
//LCD SCL-A5 SDA-A4
#define I2C_ADDR 0x27
#define BACKLIGHT_PIN 3
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin, BACKLIGHT_PIN, POSITIVE);
LCD *myLCD = &lcd;
//
//Relay
# define RelayMotPin 5
# define RelayHeatPin 6
//
// Encoder Pin definitions...sbagliato
//
#define encoder0PinA 2
#define encoder0PinB 3
//
//PID
int PIDmodes;
#define PIDWork1      0
#define PIDNotWork    1
#define PIDWorkCustom 2
#define PIDWork2      3
//readButtons

int RBmodes;
#define RBinMenu 0
#define RBinCustom 1

//Define Variables we'll be connecting to
double Input, Output;
double Setpoint;
int NewSetpoint;
//Define the aggressive and conservative Tuning Parameters
double aggKp = 2400, aggKi = 0, aggKd = 0;
double consKp = 1200, consKi = 0.00, consKd = 0.00;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
//
//Timer
int WindowSize = 6000;
unsigned long windowStartTime;
//Menu variables
MenuBackend menu = MenuBackend(menuUsed, menuChanged);
//initialize menuitems
MenuItem Pla = MenuItem("PLA");
MenuItem PlaSubItem1 = MenuItem("PLA2");
MenuItem PlaSubItem2 = MenuItem("PLA3");
MenuItem Abs = MenuItem("ABS");
MenuItem AbsSubItem1 = MenuItem("ABS2");
MenuItem AbsSubItem2 = MenuItem("ABS3");
MenuItem Custom = MenuItem("Custom");
MenuItem CustomSubItem1 = MenuItem("Custom2");
MenuItem CustomSubItem2 = MenuItem("Custom3");
MenuItem CustomSubItem3 = MenuItem("Custom4");
//
// LED
int ledPinB = 13;
int ledPinG = 12;
int ledPinR = 11;
//

ClickEncoder *encoder;
int16_t last, value;

void timerIsr() {
	encoder->service();
}

//thermistor
#define THERMISTORPIN A3
#define NUMSAMPLES 150

//temp variable
double steinhart;

int samples[NUMSAMPLES];
void setup() {

	//LED
	pinMode(ledPinR, OUTPUT);   // For the LED
	pinMode(ledPinG, OUTPUT);   // For the LED
	pinMode(ledPinB, OUTPUT);   // For the LED
								// Encoder Setup
								//
	encoder = new ClickEncoder(A1, A0, A2);//CLK-A0 DT-A1 SW-A2
	Timer1.initialize(1000);
	Timer1.attachInterrupt(timerIsr);
	last = -1;
	//
	// LCD Menu Setup
	//
	lcd.begin(16, 2);
	menu.toRoot();
	//relay
	pinMode(RelayMotPin, OUTPUT);
	pinMode(RelayHeatPin, OUTPUT);

	//Setup Thermistor Converter
	ThermoConverter.SetConversionType(Table);
	ThermoConverter.SetSamplesNumber(NUMSAMPLES);
	ThermoConverter.SetThermistorPin(THERMISTORPIN);
}
void loop()
{
	//
	PIDrun();
	//
	MenuSetup();
	//Buttons/Encoder
	readButtons();
	//
}

void MenuSetup() {
	//configure menu
	menu.getRoot().add(Pla);
	//setup the presetmode menu item
	Pla.addAfter(Abs);
	Abs.addBefore(Pla);	//we want looping both up and down
	Abs.addAfter(Custom);
	Custom.addBefore(Abs);
	Custom.addAfter(Pla);
	Pla.addBefore(Custom);

	Pla.addRight(PlaSubItem1);
	PlaSubItem1.addRight(PlaSubItem2);
	Abs.addRight(AbsSubItem1);
	AbsSubItem1.addRight(AbsSubItem2);
	Custom.addRight(CustomSubItem1);
	CustomSubItem1.addRight(CustomSubItem2);
	CustomSubItem2.addRight(CustomSubItem3);
	//
}


void PIDsetup() {

	analogReference(EXTERNAL);

	//Timer
	windowStartTime = millis();
	//
	//PID
	myPID.SetOutputLimits(0, WindowSize);//tell the PID to range between 0 and the full window size
										 //Serial.begin(9600); //apro una connessione seriale
	pinMode(RelayHeatPin, OUTPUT); //CONTROLLARE SE QUESTO SERVE
								   //inizializzo le variabili
	Input = steinhart;
	myPID.SetMode(AUTOMATIC);

	lcd.print("     doing     ");
	lcd.setCursor(0, 1);
	lcd.print("  P I D setup   ");
	delay(2500);
	lcd.clear();

}

void PIDrun() {

	switch (PIDmodes) {

	case PIDWork1:
	{		
		steinhart = ThermoConverter.ReadTemperature();  //legge la misura
		delay(1000);

		//PID
		Input = steinhart;
		double gap = abs(Setpoint - Input); //distance away from setpoint
		if (gap < 2)
		{ //we're close to setpoint, use conservative tuning parameters
			myPID.SetTunings(consKp, consKi, consKd);
		}
		else
		{
			//we're far from setpoint, use aggressive tuning parameters
			myPID.SetTunings(aggKp, aggKi, aggKd);
		}
		myPID.Compute(); //effettua il calcolo
		digitalWrite(RelayHeatPin, Output); //passa il risultato all'attuatore. NB: il risultato è comreso tra 0 e 255 di default
		if (millis() - windowStartTime > WindowSize)
		{ //time to shift the Relay Window
			windowStartTime += WindowSize;
		}
		//
		//Heater
		digitalWrite(RelayHeatPin, HIGH); //chiude il relay
		delay(Output);
		digitalWrite(RelayHeatPin, LOW);
		delay(WindowSize - Output);

		//LCD
		lcd.setCursor(0, 1);
		lcd.print(Input, 2);
		lcd.setCursor(6, 1);
		lcd.print("/");
		lcd.setCursor(7, 1);
		lcd.print(Setpoint, 2);
		lcd.setCursor(12, 1);
		lcd.print("C");
		if (gap < 1.75) {
			lcd.clear();
			delay(10);
			lcd.setCursor(0, 0);
			lcd.print("heating done!");
			lcd.setCursor(0, 1);
			lcd.print(Input, 2);
			lcd.setCursor(6, 1);
			lcd.print("/");
			lcd.setCursor(7, 1);
			lcd.print(Setpoint, 2);
			lcd.setCursor(12, 1);
			lcd.print("C");
			delay(1500);
			lcd.clear();
			delay(10);
			lcd.setCursor(0, 0);
			lcd.print("   Click for   ");
			lcd.setCursor(0, 1);
			lcd.print("    extrude   ");
			delay(1500);
		}
	}
	break;
	case PIDWork2:
	{
		steinhart = ThermoConverter.ReadTemperature();  //legge la misura
		delay(1000);

		//PID
		Input = steinhart;
		double gap = abs(Setpoint - Input); //distance away from setpoint
		if (gap < 2)
		{ //we're close to setpoint, use conservative tuning parameters
			myPID.SetTunings(consKp, consKi, consKd);
		}
		else
		{
			//we're far from setpoint, use aggressive tuning parameters
			myPID.SetTunings(aggKp, aggKi, aggKd);
		}
		myPID.Compute(); //effettua il calcolo
		digitalWrite(RelayHeatPin, Output); //passa il risultato all'attuatore. NB: il risultato è comreso tra 0 e 255 di default
		if (millis() - windowStartTime > WindowSize)
		{ //time to shift the Relay Window
			windowStartTime += WindowSize;
		}
		//
		//Heater
		digitalWrite(RelayHeatPin, HIGH); //chiude il relay
		delay(Output);
		digitalWrite(RelayHeatPin, LOW);
		delay(WindowSize - Output);

		//LCD
		lcd.setCursor(0, 1);
		lcd.print(Input, 2);
		lcd.setCursor(6, 1);
		lcd.print("/");
		lcd.setCursor(7, 1);
		lcd.print(Setpoint, 2);
		lcd.setCursor(12, 1);
		lcd.print("C");
		//if (gap < 10) {
		digitalWrite(RelayMotPin, HIGH);
		//} else {
		//digitalWrite (RelayMotPin, LOW);
		//}
	}
	break;
	//


	case PIDNotWork:
	{
		digitalWrite(RelayMotPin, LOW);
		digitalWrite(RelayHeatPin, LOW);
		//PIDNotWork non fa nulla
	}
	break;
	case PIDWorkCustom:
	{

		lcd.setCursor(0, 0);
		lcd.print("Setpoint:");
		lcd.setCursor(10, 0);
		lcd.print(Setpoint, 2);
		lcd.setCursor(0, 1);
		lcd.print("Click for OK");


		if (Setpoint > 300) {
			Setpoint = 300;
			lcd.clear();
			delay(10);
			lcd.setCursor(0, 0);
			lcd.print("Setpoint:");
			lcd.setCursor(10, 0);
			lcd.print("MaxSet");
			lcd.setCursor(0, 1);
			lcd.print("Click for OK");
		}

	}


	}
}
void menuChanged(MenuChangeEvent changed) {

	MenuItem newMenuItem = changed.to; //get the destination menu


	lcd.setCursor(0, 1); //set the start position for lcd printing to the second row

	if (newMenuItem.getName() == menu.getRoot()) {
		lcd.clear();
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("   Felfil EVO   ");
		lcd.setCursor(0, 1);
		lcd.print(" filament maker ");
		digitalWrite(ledPinR, LOW);
		digitalWrite(ledPinG, LOW);
		digitalWrite(ledPinB, LOW);
		PIDmodes = PIDNotWork;
		delay(3500);
		menu.use();
	}
	else if (newMenuItem.getName() == "PLA") {
		PIDmodes = PIDNotWork;
		lcd.clear();
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("      PLA       ");
		lcd.setCursor(0, 1);
		lcd.print("    filament   ");
	}
	else if (newMenuItem.getName() == "PLA2") {
		lcd.clear();
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("   Click for   ");
		lcd.setCursor(0, 1);
		lcd.print("    heating   ");

		/*
		if (Setpoint = steinhart + - 1.75) {
		lcd.clear();
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("    heating   ");
		lcd.setCursor(0, 1);
		lcd.print("    done!   ");
		delay(3000);
		lcd.clear();
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("   Click for   ");
		lcd.setCursor(0, 1);
		lcd.print("    extrude   ");
		}
		*/
	}
	else if (newMenuItem.getName() == "PLA3") {
		//
	}
	else if (newMenuItem.getName() == "ABS") {
		lcd.clear();
		PIDmodes = PIDNotWork;
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("      ABS       ");
		lcd.setCursor(0, 1);
		lcd.print("    filament   ");
	}
	else if (newMenuItem.getName() == "ABS2") {
		lcd.clear();
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("   Click for   ");
		lcd.setCursor(0, 1);
		lcd.print("    extrude   ");
		/*
		if (Setpoint = steinhart + - 1.75) {
		lcd.clear();
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("    heating   ");
		lcd.setCursor(0, 1);
		lcd.print("    done!   ");
		delay(3000);
		lcd.clear();
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("   Click for   ");
		lcd.setCursor(0, 1);
		lcd.print("    extrude   ");
		}
		*/
	}
	else if (newMenuItem.getName() == "ABS3") {
		//
	}
	else if (newMenuItem.getName() == "Custom") {
		lcd.clear();
		PIDmodes = PIDNotWork;
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("     Custom     ");
		lcd.setCursor(0, 1);
		lcd.print("    filament    ");
	}
	else if (newMenuItem.getName() == "Custom2") {
		lcd.clear();
		delay(10);
		lcd.setCursor(0, 0);
		lcd.print("  Click for  ");
		lcd.setCursor(0, 1);
		lcd.print("  New setpoint  ");
		PIDmodes = PIDNotWork;

	}
	else if (newMenuItem.getName() == "Custom3") {
		lcd.clear();
		delay(10);
	}

	else if (newMenuItem.getName() == "Custom4") {
		//
	}
}

//
// When the button is pushed it, blinks the LED, sets the Current menu and shows it on the Display
//
void menuUsed(MenuUseEvent used) {
	lcd.clear();
	delay(10);

	MenuItem choosenMenuItem = used.item;
	if (choosenMenuItem.getName() == menu.getRoot()) {

		lcd.clear();
		delay(50);
		lcd.setCursor(0, 0);
		lcd.print("What do you want");
		lcd.setCursor(0, 1);
		lcd.print(" extrude today? ");
		digitalWrite(ledPinR, LOW);
		digitalWrite(ledPinG, LOW);
		digitalWrite(ledPinB, LOW);
		PIDmodes = PIDNotWork;
		RBmodes = RBinMenu;
		digitalWrite(RelayMotPin, LOW);
		digitalWrite(RelayHeatPin, LOW);


	}
	else if (choosenMenuItem.getName() == "PLA") {

		delay(1000);
		PIDmodes = PIDNotWork;
		RBmodes = RBinMenu;
	}
	else if (choosenMenuItem.getName() == "PLA2") {
		PIDsetup();
		delay(1000);
		PIDmodes = PIDWork1;
		Setpoint = 160;
		digitalWrite(ledPinR, HIGH);
		digitalWrite(ledPinG, HIGH);
		digitalWrite(ledPinB, LOW);
		RBmodes = RBinMenu;
	}
	else if (choosenMenuItem.getName() == "PLA3") {
		PIDmodes = PIDWork2;
		RBmodes = RBinMenu;
		//
	}
	else if (choosenMenuItem.getName() == "ABS") {
		delay(1000);
		PIDmodes = PIDNotWork;
		RBmodes = RBinMenu;
	}
	else if (choosenMenuItem.getName() == "ABS2") {
		PIDsetup();
		delay(1000);
		PIDmodes = PIDWork1;
		Setpoint = 185;
		digitalWrite(ledPinG, HIGH);
		digitalWrite(ledPinR, LOW);
		digitalWrite(ledPinB, HIGH);
		RBmodes = RBinMenu;
	}
	else if (choosenMenuItem.getName() == "ABS3") {
		PIDmodes = PIDWork2;
		RBmodes = RBinMenu;
	}
	else if (choosenMenuItem.getName() == "Custom") {
		delay(1000);
		PIDmodes = PIDNotWork;
		RBmodes = RBinMenu;
	}
	else if (choosenMenuItem.getName() == "Custom2") {
		RBmodes = RBinCustom;
		Setpoint = 150;
		PIDmodes = PIDWorkCustom;
	}
	else if (choosenMenuItem.getName() == "Custom3") {
		PIDsetup();
		delay(1000);
		PIDmodes = PIDWork1;
		digitalWrite(ledPinB, HIGH);
		digitalWrite(ledPinG, LOW);
		digitalWrite(ledPinR, HIGH);
		RBmodes = RBinMenu;

	}
	else if (choosenMenuItem.getName() == "Custom4") {
		PIDmodes = PIDWork2;
		RBmodes = RBinMenu;
	}
}

void  readButtons() {            //read buttons status

	switch (RBmodes) {

	case RBinMenu:

	{

		int prevalue = 100;

		value = prevalue + encoder->getValue();

		if (value != last) {


			last = value;
			//Serial.print("Encoder Value: ");
			//Serial.println(value);
			/*
			#ifdef WITH_LCD
			lcd.setCursor(0, 0);
			lcd.print("         ");
			lcd.setCursor(0, 0);
			lcd.print(value);
			#endif
			*/
		}

		if (value <= 99) {

			menu.moveUp();
		}
		if (value >= 101) {
			menu.moveDown();
		}
		if (value = prevalue) {
			menu.getCurrent();
		}




		ClickEncoder::Button b = encoder->getButton();
		if (b != ClickEncoder::Open) {

#define VERBOSECASE(label) case label: /*Serial.println(#label);*/ break;
			;



			switch (b) {
				VERBOSECASE(ClickEncoder::Pressed);
				VERBOSECASE(ClickEncoder::Released)
			case ClickEncoder::DoubleClicked:
				menu.moveLeft();
				break;
			case ClickEncoder::Clicked:
				menu.use();
				menu.moveRight();
				break;
			case ClickEncoder::Held:
				menu.toRoot();
				digitalWrite(RelayMotPin, LOW);
				digitalWrite(RelayHeatPin, LOW);
				break;
			}
		}
	}
	break;
	//
	case RBinCustom:

	{
		value += encoder->getValue();

		if (value != last) {


			last = value;
			Setpoint = 125 + (value * 0.25);
		}





		ClickEncoder::Button b = encoder->getButton();
		if (b != ClickEncoder::Open) {

#define VERBOSECASE(label) case label: /*Serial.println(#label);*/ break;



			switch (b) {
				VERBOSECASE(ClickEncoder::Pressed);
				VERBOSECASE(ClickEncoder::Released)
			case ClickEncoder::DoubleClicked:
				menu.moveLeft();
				break;
			case ClickEncoder::Clicked:
				menu.use();
				menu.moveRight();
				break;
			case ClickEncoder::Held:
				menu.toRoot();
				break;
			}
		}
	}
	}
}

