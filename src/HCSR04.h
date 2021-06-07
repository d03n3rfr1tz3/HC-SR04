/*
  HCSR04 - Library for arduino, for HC-SR04 ultrasonic distance sensor.
  Created by Dirk Sarodnick, 2020.
*/

#ifndef HCSR04_H
#define HCSR04_H

#include "Arduino.h"

#define HCSR04_INVALID_RESULT  -1;
#define HCSR04_NO_TRIGGER      -2;
#define HCSR04_NO_ECHO         -3;

class HCSR04Sensor {
	public:
		HCSR04Sensor();
		~HCSR04Sensor();

		typedef enum eUltraSonicUnlock {
			unlockSkip = 0,
			unlockMaybe = 1,
			unlockForced = 2
		} eUltraSonicUnlock_t;
		
		void begin(uint8_t triggerPin, uint8_t echoPin) { begin(triggerPin, new uint8_t[1]{ echoPin }, 1); }
		void begin(uint8_t triggerPin, uint8_t* echoPins, uint8_t echoCount) { begin(triggerPin, echoPins, echoCount, 100000, eUltraSonicUnlock_t::unlockSkip); }
		void begin(uint8_t triggerPin, uint8_t echoPin, uint32_t timeout, eUltraSonicUnlock_t unlock) { begin(triggerPin, new uint8_t[1]{ echoPin }, 1, timeout, unlock); }
		void begin(uint8_t triggerPin, uint8_t* echoPins, uint8_t echoCount, uint32_t timeout, eUltraSonicUnlock_t unlock) { begin(triggerPin, echoPins, echoCount, timeout, 10, 10, unlock); }
		void begin(uint8_t triggerPin, uint8_t* echoPins, uint8_t echoCount, uint32_t timeout, uint16_t triggerTime, uint16_t triggerWait, eUltraSonicUnlock_t unlock);
		void end();
		
		long* measureMicroseconds() { measureMicroseconds(lastMicroseconds); return lastMicroseconds; }
		void measureMicroseconds(long* results);

		double* measureDistanceMm() { measureDistanceMm(defaultTemperature, lastDistances); return lastDistances; }
		void measureDistanceMm(double* results) { measureDistanceMm(defaultTemperature, results == NULL ? lastDistances : results); }
		double* measureDistanceMm(float temperature) { measureDistanceMm(temperature, lastDistances); return lastDistances; }
		void measureDistanceMm(float temperature, double* results);

		double* measureDistanceCm() { measureDistanceCm(defaultTemperature, lastDistances); return lastDistances; }
		void measureDistanceCm(double* results) { measureDistanceCm(defaultTemperature, results == NULL ? lastDistances : results); }
		double* measureDistanceCm(float temperature) { measureDistanceCm(temperature, lastDistances); return lastDistances; }
		void measureDistanceCm(float temperature, double* results);

		double* measureDistanceIn() { measureDistanceIn(defaultTemperature, lastDistances); return lastDistances; }
		void measureDistanceIn(double* results) { measureDistanceIn(defaultTemperature, results == NULL ? lastDistances : results); }
		double* measureDistanceIn(float temperature) { measureDistanceIn(temperature, lastDistances); return lastDistances; }
		void measureDistanceIn(float temperature, double* results);
		
		static void triggerInterrupt0(void);
		static void triggerInterrupt1(void);
		static void triggerInterrupt2(void);
		static void triggerInterrupt3(void);
		static void triggerInterrupt4(void);
		static void triggerInterrupt5(void);
		static void triggerInterrupt6(void);
		static void triggerInterrupt7(void);
		static void triggerInterrupt8(void);
		static void triggerInterrupt9(void);
		
		static void echoInterrupt0(void);
		static void echoInterrupt1(void);
		static void echoInterrupt2(void);
		static void echoInterrupt3(void);
		static void echoInterrupt4(void);
		static void echoInterrupt5(void);
		static void echoInterrupt6(void);
		static void echoInterrupt7(void);
		static void echoInterrupt8(void);
		static void echoInterrupt9(void);
	
	private:
		float defaultTemperature = 19.307;
		long* lastMicroseconds;
		double* lastDistances;

		uint32_t timeout;
		uint16_t triggerTime = 10; // HC-SR04 needs at least 10µs trigger. Others may need longer trigger pulses.
		uint16_t triggerWait = 10; // HC-SR04 sends its signal about 200µs. We only wait a small amount to reduce interference, but to not miss anything on slower clock speeds.
		volatile uint8_t triggerPin;
		volatile unsigned long* volatile triggerTimes;
		
		uint8_t echoCount;
		volatile int16_t* volatile echoStages;
		volatile int16_t* volatile echoInts;
		volatile int16_t* volatile echoPorts;
		volatile unsigned long* volatile echoTimes;
		
		void triggerInterrupt(uint8_t);
		void echoInterrupt(uint8_t);
		void unlockSensors(eUltraSonicUnlock_t, uint8_t*);
};

extern HCSR04Sensor HCSR04;

#endif // HCSR04_H
