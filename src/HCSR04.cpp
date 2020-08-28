/*
  HCSR04 - Library for arduino, for HC-SR04 ultrasonic distance sensor.
  Created by Dirk Sarodnick, 2020.
*/

#include "Arduino.h"
#include "HCSR04.h"

HCSR04Sensor::HCSR04Sensor() {}
HCSR04Sensor::~HCSR04Sensor() { this->end(); }

void HCSR04Sensor::begin(int triggerPin, int* echoPins, short echoCount, int timeout, eUltraSonicUnlock_t unlock) {
	if (this->echoCount != echoCount) this->end();
	
	this->triggerPin = triggerPin;
	pinMode(triggerPin, OUTPUT);

	this->timeout = timeout;
	this->echoCount = echoCount;
	
	if (this->lastMicroseconds == NULL) this->lastMicroseconds = new unsigned long[echoCount];
	if (this->lastDistances == NULL) this->lastDistances = new double[echoCount];
	
	if (this->triggerTimes == NULL) this->triggerTimes = new unsigned long[echoCount];
	if (this->echoTimes == NULL) this->echoTimes = new unsigned long[echoCount];

	if (this->echoPins == NULL) this->echoPins = new int[echoCount];
	for (int i = 0; i < this->echoCount; i++) {
		this->triggerTimes[i] = 0;
		this->echoTimes[i] = 0;
		this->echoPins[i] = digitalPinToInterrupt(echoPins[i]);
		pinMode(echoPins[i], INPUT);
	}
	
	// Unlock sensors that are possibly in a locked state, if this feature is enabled.
	this->unlockSensors(unlock, echoPins);
}

void HCSR04Sensor::end() {
	if (this->lastMicroseconds != NULL) delete []this->lastMicroseconds;
	if (this->lastDistances != NULL) delete []this->lastDistances;
	if (this->triggerTimes != NULL) delete []this->triggerTimes;
	if (this->echoTimes != NULL) delete []this->echoTimes;
	if (this->echoPins != NULL) delete []this->echoPins;
	
	this->lastMicroseconds = NULL;
	this->lastDistances = NULL;
	this->triggerTimes = NULL;
	this->echoTimes = NULL;
	this->echoPins = NULL;
}

void HCSR04Sensor::measureMicroseconds(unsigned long* results) {
	if (results == NULL) results = this->lastMicroseconds;

	unsigned long startMicros = micros();
	
	// Make sure that trigger pin is LOW.
	digitalWrite(triggerPin, LOW);
	delayMicroseconds(2);
	
	// Hold trigger for 10 microseconds, which is signal for sensor to measure distance.
	digitalWrite(triggerPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(triggerPin, LOW);
	
	// Attach interrupts to echo pins for the starting point
	for (int i = 0; i < this->echoCount; i++) {
		switch (i) {
			case 0: attachInterrupt(this->echoPins[i], &triggerInterrupt0, RISING); break;
			case 1: attachInterrupt(this->echoPins[i], &triggerInterrupt1, RISING); break;
			case 2: attachInterrupt(this->echoPins[i], &triggerInterrupt2, RISING); break;
			case 3: attachInterrupt(this->echoPins[i], &triggerInterrupt3, RISING); break;
			case 4: attachInterrupt(this->echoPins[i], &triggerInterrupt4, RISING); break;
			case 5: attachInterrupt(this->echoPins[i], &triggerInterrupt5, RISING); break;
			case 6: attachInterrupt(this->echoPins[i], &triggerInterrupt6, RISING); break;
			case 7: attachInterrupt(this->echoPins[i], &triggerInterrupt7, RISING); break;
			case 8: attachInterrupt(this->echoPins[i], &triggerInterrupt8, RISING); break;
			case 9: attachInterrupt(this->echoPins[i], &triggerInterrupt9, RISING); break;
		}
	}
	
	// Wait until all echos are returned or timed out.
	while(1) {
		delayMicroseconds(1);
		
		bool finished = true;
		bool waiting = true;
		
		unsigned long currentMicros = micros();
		unsigned long elapsedMicros = currentMicros - startMicros;
		for (int i = 0; i < this->echoCount; i++) {
			unsigned long elapsedMicrosI = currentMicros - this->triggerTimes[i];
			waiting &= elapsedMicros < this->timeout || (this->triggerTimes[i] > 0 && elapsedMicrosI < this->timeout);
			
			if (this->triggerTimes[i] > 0 || !waiting) {
				switch (i) {
					case 0: detachInterrupt(this->echoPins[i]);
					case 1: detachInterrupt(this->echoPins[i]);
					case 2: detachInterrupt(this->echoPins[i]);
					case 3: detachInterrupt(this->echoPins[i]);
					case 4: detachInterrupt(this->echoPins[i]);
					case 5: detachInterrupt(this->echoPins[i]);
					case 6: detachInterrupt(this->echoPins[i]);
					case 7: detachInterrupt(this->echoPins[i]);
					case 8: detachInterrupt(this->echoPins[i]);
					case 9: detachInterrupt(this->echoPins[i]);
				}
			} else finished &= false;

			if (this->triggerTimes[i] > 0) {
				switch (i) {
				case 0: attachInterrupt(this->echoPins[i], &echoInterrupt0, FALLING); break;
				case 1: attachInterrupt(this->echoPins[i], &echoInterrupt1, FALLING); break;
				case 2: attachInterrupt(this->echoPins[i], &echoInterrupt2, FALLING); break;
				case 3: attachInterrupt(this->echoPins[i], &echoInterrupt3, FALLING); break;
				case 4: attachInterrupt(this->echoPins[i], &echoInterrupt4, FALLING); break;
				case 5: attachInterrupt(this->echoPins[i], &echoInterrupt5, FALLING); break;
				case 6: attachInterrupt(this->echoPins[i], &echoInterrupt6, FALLING); break;
				case 7: attachInterrupt(this->echoPins[i], &echoInterrupt7, FALLING); break;
				case 8: attachInterrupt(this->echoPins[i], &echoInterrupt8, FALLING); break;
				case 9: attachInterrupt(this->echoPins[i], &echoInterrupt9, FALLING); break;
				}
			}
			else finished &= false;
			
			if (this->echoTimes[i] > 0 || !waiting) {
				switch (i) {
					case 0: detachInterrupt(this->echoPins[i]); break;
					case 1: detachInterrupt(this->echoPins[i]); break;
					case 2: detachInterrupt(this->echoPins[i]); break;
					case 3: detachInterrupt(this->echoPins[i]); break;
					case 4: detachInterrupt(this->echoPins[i]); break;
					case 5: detachInterrupt(this->echoPins[i]); break;
					case 6: detachInterrupt(this->echoPins[i]); break;
					case 7: detachInterrupt(this->echoPins[i]); break;
					case 8: detachInterrupt(this->echoPins[i]); break;
					case 9: detachInterrupt(this->echoPins[i]); break;
				}
			} else finished &= false;
		}
		
		if (!waiting || finished) break;
	}
	
	// Determine the durations of each sensor.
	for (int i = 0; i < this->echoCount; i++) {
		if (this->triggerTimes[i] > 0 && this->echoTimes[i] > 0) {
			results[i] = this->echoTimes[i] - this->triggerTimes[i];
		} else if (this->triggerTimes[i] > 0) {
			results[i] = -1;
		} else {
			results[i] = -2;
		}

		this->triggerTimes[i] = 0;
		this->echoTimes[i] = 0;
	}
}

void HCSR04Sensor::measureDistanceMm(float temperature, double* results) {
	if (results == NULL) results = this->lastDistances;

	double speedOfSoundInMmPerMs = (331.3 + 0.606 * temperature) / 1000 / 100; // Cair ≈ (331.3 + 0.606 ⋅ ϑ) m/s
	unsigned long* times = measureMicroseconds();
	
	// Calculate the distance in mm for each result.
	for (int i = 0; i < this->echoCount; i++) {
		double distanceCm = times[i] / 2.0 * speedOfSoundInMmPerMs;
		if (distanceCm < 1 || distanceCm > 400) {
			results[i] = -1.0;
		} else {
			results[i] = distanceCm;
		}
	}
}

void HCSR04Sensor::measureDistanceCm(float temperature, double* results) {
	if (results == NULL) results = this->lastDistances;

	double speedOfSoundInCmPerMs = (331.3 + 0.606 * temperature) / 1000 / 10; // Cair ≈ (331.3 + 0.606 ⋅ ϑ) m/s
	unsigned long* times = measureMicroseconds();
	
	// Calculate the distance in cm for each result.
	for (int i = 0; i < this->echoCount; i++) {
		double distanceCm = times[i] / 2.0 * speedOfSoundInCmPerMs;
		if (distanceCm < 1 || distanceCm > 400) {
			results[i] = -1.0;
		} else {
			results[i] = distanceCm;
		}
	}
}

void HCSR04Sensor::measureDistanceIn(float temperature, double* results) {
	if (results == NULL) results = this->lastDistances;

	double speedOfSoundInCmPerMs = (331.3 + 0.606 * temperature) * 39.37007874 / 1000 / 10; // Cair ≈ (331.3 + 0.606 ⋅ ϑ) m/s
	unsigned long* times = measureMicroseconds();

	// Calculate the distance in cm for each result.
	for (int i = 0; i < this->echoCount; i++) {
		double distanceCm = times[i] / 2.0 * speedOfSoundInCmPerMs;
		if (distanceCm < 1 || distanceCm > 400) {
			results[i] = -1.0;
		}
		else {
			results[i] = distanceCm;
		}
	}
}

void HCSR04Sensor::unlockSensors(eUltraSonicUnlock_t unlock, int* echoPins) {
	if (unlock == eUltraSonicUnlock_t::unlockSkip) return;
	bool hasLocked = false;

	// Check if any sensor is in a locked state and unlock it if necessary.
	for (int i = 0; echoPins[i] != 0; i++) {
		if (unlock == eUltraSonicUnlock_t::unlockMaybe && digitalRead(echoPins[i]) == LOW) continue;
		pinMode(echoPins[i], OUTPUT);
		digitalWrite(echoPins[i], LOW);
		hasLocked = true;
	}
	
	if (hasLocked) delay(100);

	// Revert the pinMode after potential unlocking.
	for (int i = 0; echoPins[i] != 0; i++) {
		pinMode(echoPins[i], INPUT);
	}
	
	if (hasLocked) delay(100);
}

void HCSR04Sensor::triggerInterrupt(int index) {
	if (this->triggerTimes[index] == 0) this->triggerTimes[index] = micros();
}

void HCSR04Sensor::echoInterrupt(int index) {
	if (this->echoTimes[index] == 0) this->echoTimes[index] = micros();
}

void HCSR04Sensor::triggerInterrupt0() { HCSR04.triggerInterrupt(0); }
void HCSR04Sensor::triggerInterrupt1() { HCSR04.triggerInterrupt(1); }
void HCSR04Sensor::triggerInterrupt2() { HCSR04.triggerInterrupt(2); }
void HCSR04Sensor::triggerInterrupt3() { HCSR04.triggerInterrupt(3); }
void HCSR04Sensor::triggerInterrupt4() { HCSR04.triggerInterrupt(4); }
void HCSR04Sensor::triggerInterrupt5() { HCSR04.triggerInterrupt(5); }
void HCSR04Sensor::triggerInterrupt6() { HCSR04.triggerInterrupt(6); }
void HCSR04Sensor::triggerInterrupt7() { HCSR04.triggerInterrupt(7); }
void HCSR04Sensor::triggerInterrupt8() { HCSR04.triggerInterrupt(8); }
void HCSR04Sensor::triggerInterrupt9() { HCSR04.triggerInterrupt(9); }

void HCSR04Sensor::echoInterrupt0() { HCSR04.echoInterrupt(0); }
void HCSR04Sensor::echoInterrupt1() { HCSR04.echoInterrupt(1); }
void HCSR04Sensor::echoInterrupt2() { HCSR04.echoInterrupt(2); }
void HCSR04Sensor::echoInterrupt3() { HCSR04.echoInterrupt(3); }
void HCSR04Sensor::echoInterrupt4() { HCSR04.echoInterrupt(4); }
void HCSR04Sensor::echoInterrupt5() { HCSR04.echoInterrupt(5); }
void HCSR04Sensor::echoInterrupt6() { HCSR04.echoInterrupt(6); }
void HCSR04Sensor::echoInterrupt7() { HCSR04.echoInterrupt(7); }
void HCSR04Sensor::echoInterrupt8() { HCSR04.echoInterrupt(8); }
void HCSR04Sensor::echoInterrupt9() { HCSR04.echoInterrupt(9); }

HCSR04Sensor HCSR04;
