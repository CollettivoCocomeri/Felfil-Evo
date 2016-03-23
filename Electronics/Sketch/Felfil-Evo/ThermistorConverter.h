// ThermoConverter.h

#ifndef _THERMOCONVERTER_h
#define _THERMOCONVERTER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

enum ConversionType { Table, Dynamic };

class ThermistorConverter
{
protected:
	uint8_t _thermistorPin;
	uint8_t _samplingInterval;
	uint8_t _samplesNumber;
	ConversionType _conversionType;

	float ConvertTemperatureDynamic(float input);
	float ConvertTemperatureTable(float input);
	float ConvertTemperature(float input);

public:
	ThermistorConverter();

	void SetThermistorPin(uint8_t pin);
	void SetSamplingInterval(uint8_t value);
	void SetSamplesNumber(uint8_t value);
	void SetConversionType(ConversionType conversionType);

	float ReadTemperature();
};

extern ThermistorConverter ThermoConverter;

#endif

