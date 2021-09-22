#ifndef _FELFILREADER_h
#define _FELFILREADER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Max6675.h>

#define DefaultCurrentSamples 1000
#define DefaultTemperatureSamples 100

class FelfilReader
{
  protected:
  	Max6675* ts;
  	uint8_t engineCurrentPin;
  
  	uint16_t engineCurrentSamples;
  	uint16_t engineCurrentSampleIndex = 0;
  	uint16_t engineCurrentSamplePeriod;
  	unsigned long lastEngineCurrentSampleTime = 0;
  	float engineCurrentAvg = 0;
  	float lastEngineCurrentMeasure = 0;
  	bool engineCurrentInitialized = false;
  
  	uint16_t temperatureSamples;
  	uint16_t temperatureSamplePeriod;
  	uint16_t temperatureSampleIndex = 0;
  	unsigned long lastTemparatureSampleTime = 0;
  	float temperatureAvg = 0;
  	float lastTemperatureMeasure = 0;
  	bool temperatureInitialized = false;
  
  	void SampleEngineCurrent(float offset);
  	void SampleTemperature();
  
  public:
    int temp_start = 0;
    int temp_end   = 0;
    
  	FelfilReader(Max6675* ts, uint8_t engineCurrentPin);
  	void SetupEngineCurrentSamples(uint16_t tot, uint16_t samplingPeriod);
  	void SetupTemperatureSamples(uint16_t tot, uint16_t samplingPeriod);
  
  	float ReadEngineCurrent(float offset);
  	float ReadTemperature();
    
    void setTempStart();
    long int getTempStart();
    void setTempEnd();
    long int getTempEnd();
  
  	bool IsInitialized();
};
#endif
