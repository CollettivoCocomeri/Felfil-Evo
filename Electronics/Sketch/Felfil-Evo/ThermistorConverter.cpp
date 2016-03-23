
#include "ThermistorConverter.h"

#define _DefaultConversionType Table;
#define _DetaultSamplingInterval 10;
#define _DefaultSamplesNumber 10;
#define _DefaultThermistorPin A0; 

#pragma region Common

float InterpolatePoint(float x, float x1, float y1, float x2, float y2)
{
	return ((x - x1)*(y2 - y1) / (x2 - x1)) + y1;
}

#pragma endregion

#pragma region Table Conversion

// EPCOS 100K Thermistor #3(B57560G104F)
// Made with createTemperatureLookup.py (http://svn.reprap.org/trunk/reprap/firmware/Arduino/utilities/createTemperatureLookup.py)
// ./createTemperatureLookup.py --r0=100000 --t0=25 --r1=0 --r2=4700 --beta=4036 --max-adc=1023
// r0: 100000
// t0: 25
// r1: 0
// r2: 4700
// beta: 4036
// max adc: 1023
#define NUMTEMPS 20
// {ADC, temp }, // temp
const uint16_t temptable[NUMTEMPS][2] = {
	{ 1, 864 }, // 864.165363324 C
	{ 54, 258 }, // 258.53991594 C
	{ 107, 211 }, // 211.310066205 C
	{ 160, 185 }, // 185.861725716 C
	{ 213, 168 }, // 168.31793816 C
	{ 266, 154 }, // 154.754297589 C
	{ 319, 143 }, // 143.52544406 C
	{ 372, 133 }, // 133.784751118 C
	{ 425, 125 }, // 125.033500921 C
	{ 478, 116 }, // 116.945124847 C
	{ 531, 109 }, // 109.283980973 C
	{ 584, 101 }, // 101.861768746 C
	{ 637, 94 }, // 94.5095302806 C
	{ 690, 87 }, // 87.0542728805 C
	{ 743, 79 }, // 79.2915563492 C
	{ 796, 70 }, // 70.9409729952 C
	{ 849, 61 }, // 61.5523326183 C
	{ 902, 50 }, // 50.25271896 C
	{ 955, 34 }, // 34.7815846664 C
	{ 1008, 2 } // 2.86606331838 C
};

const uint16_t _MinTableVoltage = temptable[0][0];
const uint16_t _MaxTableVoltage = temptable[NUMTEMPS - 1][0];


float ThermistorConverter::ConvertTemperatureTable(float input)
{
	if (input <= _MinTableVoltage)
		return temptable[0][1];
	if (input >= _MaxTableVoltage)
		return temptable[NUMTEMPS - 1][1];

	for (uint8_t i = 0; i < NUMTEMPS - 1; i++)
	{
		float x1 = temptable[i][0];
		float x2 = temptable[i + 1][0];

		if (x1 < input && input <= x2)
		{
			float y1 = temptable[i][1];
			float y2 = temptable[i + 1][1];
			return InterpolatePoint(input, x1, y1, x2, y2);
		}
	}

	return -1;
}

#pragma endregion

#pragma region Dynamic Conversion

// resistance at 25 degrees C
#define THERMISTORNOMINAL 100000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 100000   


float ThermistorConverter::ConvertTemperatureDynamic(float input)
{
	// convert the value to resistance
	float val = 1023 / input - 1;
	val = SERIESRESISTOR / val;

	float steinhart;
	steinhart = val / THERMISTORNOMINAL;     // (R/Ro)
	steinhart = log(steinhart);                  // ln(R/Ro)
	steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
	steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
	steinhart = 1.0 / steinhart;                 // Invert
	steinhart -= 273.15;                         // convert to C

	return steinhart;
}

#pragma endregion

float ThermistorConverter::ConvertTemperature(float input)
{
	switch (_conversionType)
	{
	case Table:
		return ConvertTemperatureTable(input);
	default:
	case Dynamic:
		return ConvertTemperatureDynamic(input);
	}
}

ThermistorConverter::ThermistorConverter()
{
	_conversionType = _DefaultConversionType;
	_thermistorPin = _DefaultThermistorPin;
	_samplingInterval = _DetaultSamplingInterval;
	_samplesNumber = _DefaultSamplesNumber;
}

void ThermistorConverter::SetThermistorPin(uint8_t pin)
{
	_thermistorPin = pin;
}

void ThermistorConverter::SetSamplingInterval(uint8_t value)
{
	_samplingInterval = value;
}

void ThermistorConverter::SetSamplesNumber(uint8_t value)
{
	_samplesNumber = value;
}

void ThermistorConverter::SetConversionType(ConversionType conversionType)
{
	_conversionType = conversionType;
}

float ThermistorConverter::ReadTemperature()
{
	int *samples = new int[_samplesNumber];
	uint8_t i;

	// take N samples in a row, with a slight delay
	for (i = 0; i< _samplesNumber; i++) {
		samples[i] = analogRead(_thermistorPin);
		delay(_samplingInterval);
	}

	// average all the samples out
	float average = 0;
	for (i = 0; i < _samplesNumber; i++) {
		average += samples[i];
	}
	average /= _samplesNumber;

	return ConvertTemperature(average);
}

ThermistorConverter ThermoConverter;