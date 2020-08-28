/*
  HCSR04 - Library for arduino, for HC-SR04 ultrasonic distance sensor.
  Created by Dirk Sarodnick, 2020.
*/

#ifndef HCSR04_H
#define HCSR04_H

#include "Arduino.h"

class HCSR04Sensor {
	public:
		HCSR04Sensor();
		~HCSR04Sensor();

		typedef enum eUltraSonicUnlock {
			unlockSkip = 0,
			unlockMaybe = 1,
			unlockForced = 2
		} eUltraSonicUnlock_t;
		
		void begin(int triggerPin, int echoPin) { begin(triggerPin, new int[1]{ echoPin }, 1); }
		void begin(int triggerPin, int* echoPins, short echoCount) { begin(triggerPin, echoPins, echoCount, 100000, eUltraSonicUnlock_t::unlockSkip); }
		void begin(int triggerPin, int echoPin, int timeout, eUltraSonicUnlock_t unlock) { begin(triggerPin, new int[1]{ echoPin }, 1, timeout, unlock); }
		void begin(int triggerPin, int* echoPins, short echoCount, int timeout, eUltraSonicUnlock_t unlock);
		void end();
		
		unsigned long* measureMicroseconds() { measureMicroseconds(lastMicroseconds); return lastMicroseconds; }
		void measureMicroseconds(unsigned long* results);

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
		unsigned long* lastMicroseconds;
		double* lastDistances;

		int timeout;
		volatile int triggerPin;
		volatile unsigned long* volatile triggerTimes;
		
		short echoCount;
		volatile int* volatile echoPins;
		volatile unsigned long* volatile echoTimes;
		
		void triggerInterrupt(int);
		void echoInterrupt(int);
		void unlockSensors(eUltraSonicUnlock_t, int*);
};

extern HCSR04Sensor HCSR04;

#endif // HCSR04_H
