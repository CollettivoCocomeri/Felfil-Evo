// 
// 
// 

#include "FelfilReader.h"

#pragma region Init

FelfilReader::FelfilReader(Max6675* ts, uint8_t engineCurrentPin)
{
	this->ts = ts;
	this->engineCurrentPin = engineCurrentPin;
}

void FelfilReader::SetupEngineCurrentSamples(uint16_t tot, uint16_t samplingPeriod)
{
	this->engineCurrentSamples = tot;
	this->engineCurrentSamplePeriod = samplingPeriod;
}

void FelfilReader::SetupTemperatureSamples(uint16_t tot, uint16_t samplingPeriod)
{
	this->temperatureSamples = tot;
	this->temperatureSamplePeriod = samplingPeriod;
}

#pragma endregion

#pragma region Refresh Sample

void FelfilReader::SampleEngineCurrent(float offset)
{
	engineCurrentAvg = 0;

	for (uint16_t i = 0; i < engineCurrentSamples; i++)
	{
		float measure = (.0264 * analogRead(engineCurrentPin) - 13.51) + offset;
		if (measure < 0)
			measure = 0;

		engineCurrentAvg += measure / engineCurrentSamples;
		delay(1);
	}
	
	lastEngineCurrentMeasure = engineCurrentAvg;
	engineCurrentInitialized = true;
}

void FelfilReader::SampleTemperature()
{
	//faccio passare almeno engineCurrentSamplePeriod ms
	unsigned long now = millis();
	if (now - lastTemparatureSampleTime < temperatureSamplePeriod)
		return;

	float measure = ts->getCelsius();

	temperatureAvg += measure / temperatureSamples;
	temperatureSampleIndex++;
	lastTemparatureSampleTime = now;

	if (temperatureSampleIndex >= temperatureSamples)
	{
		lastTemperatureMeasure = temperatureAvg;
		temperatureSampleIndex = 0;
		temperatureInitialized = true;
		temperatureAvg = 0;
	}
}

#pragma endregion

#pragma region Read Measures

float FelfilReader::ReadEngineCurrent(float offset)
{
	SampleEngineCurrent(offset);

	return lastEngineCurrentMeasure;
}

float FelfilReader::ReadTemperature()
{
	SampleTemperature();

	return lastTemperatureMeasure;
}

bool FelfilReader::IsInitialized()
{
	return temperatureInitialized && engineCurrentInitialized;
}

#pragma endregion
