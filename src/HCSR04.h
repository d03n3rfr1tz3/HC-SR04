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
		
		void begin(int triggerPin, int echoPin);
		void begin(int triggerPin, int* echoPins, short echoCount);
		void begin(int triggerPin, int echoPin, int timeout, bool unlock);
		void begin(int triggerPin, int* echoPins, short echoCount, int timeout, bool unlock);
		
		unsigned long* measureMicroseconds();
		double* measureDistanceMm();
		double* measureDistanceMm(float temperature);
		double* measureDistanceCm();
		double* measureDistanceCm(float temperature);
		double* measureDistanceIn();
		double* measureDistanceIn(float temperature);
		
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
		int timeout;
		volatile int triggerPin;
		volatile unsigned long* volatile triggerTimes;
		
		short echoCount;
		volatile int* volatile echoPins;
		volatile unsigned long* volatile echoTimes;
		
		void triggerInterrupt(int);
		void echoInterrupt(int);
		void unlockSensors(int*);
};

extern HCSR04Sensor HCSR04;

#endif // HCSR04_H
