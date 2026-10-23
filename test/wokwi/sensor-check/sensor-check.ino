/*
  Hardware check for the Wokwi run in the continuous integration. Three HC-SR04
  modules share one trigger pin and are measured in parallel, while an NTC
  thermistor supplies the ambient temperature.

  The sketch judges its own measurements and prints one machine readable verdict
  that the workflow matches against. The nominal distances belong to the diagram
  next to this sketch and have to be changed together with it.
*/

#include <HCSR04.h>
#include <math.h>
#include <string.h>

const byte triggerPin = 5;
const byte echoCount = 3;
byte echoPins[echoCount] = { 18, 19, 23 };

// The distances the diagram puts in front of the modules.
const double nominalCm[echoCount] = { 60.0, 150.0, 300.0 };

const byte thermistorPin = 34;
const float thermistorBeta = 3950.0; // has to match the beta attribute of the part
const int adcMax = 4095;             // the ESP32 reads 12 bit

const int rounds = 10;                 // a stuck handler only shows in the following measurement
const double absoluteTolerance = 0.05; // the speed of sound of the Wokwi model is unknown
const double ratioTolerance = 0.01;    // a ratio does not depend on the speed of sound
const unsigned long parallelLimitUs = 25000;
const unsigned long roundPauseMs = 100;

const char* status[echoCount] = { NULL };
bool finished = false;

// Temperature of the divider built from the thermistor and its 10k resistor.
float temperatureFromAdc(int raw) {
	if (raw < 1) raw = 1;
	if (raw > adcMax - 1) raw = adcMax - 1;

	float ratio = (float) raw / (float) (adcMax - raw);
	return 1.0 / (log(ratio) / thermistorBeta + 1.0 / 298.15) - 273.15;
}

// Name of the result a module delivered, as it appears in the verdict.
const char* statusOf(double distance) {
	if (distance == HCSR04_NO_TRIGGER) return "KEIN-TRIGGER";
	if (distance == HCSR04_NO_ECHO) return "KEIN-ECHO";
	if (distance == HCSR04_INVALID_RESULT) return "UNGUELTIG";
	return "OK";
}

void reportFailure(const char* line) {
	Serial.print("WOKWI-CI-FEHLER: ");
	Serial.println(line);
}

// Judge one measurement and hold on to the status of every module.
bool checkRound(int round, const double* distances, unsigned long elapsedUs) {
	char line[160];
	int answered = 0;

	for (int i = 0; i < echoCount; i++) {
		const char* current = statusOf(distances[i]);
		if (strcmp(current, "OK") == 0) answered++;

		// Only a working module and one without a trigger belong to the two diagrams.
		if (strcmp(current, "OK") != 0 && strcmp(current, "KEIN-TRIGGER") != 0) {
			snprintf(line, sizeof(line), "Modul %d meldet %s in Messung %d", i + 1, current, round);
			reportFailure(line);
			return false;
		}

		if (status[i] == NULL) {
			status[i] = current;
		} else if (strcmp(status[i], current) != 0) {
			snprintf(line, sizeof(line), "Modul %d wechselt in Messung %d von %s nach %s", i + 1, round, status[i], current);
			reportFailure(line);
			return false;
		}
	}

	for (int i = 0; i < echoCount; i++) {
		if (strcmp(status[i], "OK") != 0) continue;

		double deviation = fabs(distances[i] - nominalCm[i]) / nominalCm[i];
		if (deviation > absoluteTolerance) {
			snprintf(line, sizeof(line), "Modul %d misst %.2f cm statt %.2f cm in Messung %d", i + 1, distances[i], nominalCm[i], round);
			reportFailure(line);
			return false;
		}
	}

	// A ratio cancels the speed of sound out, so it may be checked much tighter.
	if (strcmp(status[0], "OK") == 0) {
		for (int i = 1; i < echoCount; i++) {
			if (strcmp(status[i], "OK") != 0) continue;

			double expected = nominalCm[i] / nominalCm[0];
			double measured = distances[i] / distances[0];
			if (fabs(measured - expected) / expected > ratioTolerance) {
				snprintf(line, sizeof(line), "Verhaeltnis von Modul %d zu Modul 1 ist %.4f statt %.4f in Messung %d", i + 1, measured, expected, round);
				reportFailure(line);
				return false;
			}
		}
	}

	// Measured in parallel the modules together take as long as the longest echo.
	if (answered == echoCount && elapsedUs > parallelLimitUs) {
		snprintf(line, sizeof(line), "Messung %d dauert %lu us statt hoechstens %lu us und laeuft damit nicht parallel", round, elapsedUs, parallelLimitUs);
		reportFailure(line);
		return false;
	}

	return true;
}

// Print the verdict the workflow matches against.
void reportVerdict(unsigned long elapsedUs, float temperature) {
	char line[160];
	bool complete = true;
	int length = snprintf(line, sizeof(line), "WOKWI-CI:");

	for (int i = 0; i < echoCount; i++) {
		if (strcmp(status[i], "OK") != 0) complete = false;
		length += snprintf(line + length, sizeof(line) - length, " %d=%s", i + 1, status[i]);
	}

	if (complete) {
		snprintf(line + length, sizeof(line) - length, " (parallel %lu us, %.1f C, %lu ms)", elapsedUs, temperature, millis());
	} else {
		snprintf(line + length, sizeof(line) - length, " (%.1f C, %lu ms)", temperature, millis());
	}

	Serial.println(line);
}

void setup() {
	Serial.begin(115200);
	HCSR04.begin(triggerPin, echoPins, echoCount);
	delay(50);

	Serial.println();
	Serial.println("HC-SR04 Hardwarepruefung auf dem ESP32");
}

void loop() {
	if (finished) {
		delay(1000);
		return;
	}
	finished = true;

	char line[160];
	float temperature = 0.0;
	unsigned long elapsedUs = 0;

	for (int round = 1; round <= rounds; round++) {
		temperature = temperatureFromAdc(analogRead(thermistorPin));

		unsigned long startedUs = micros();
		double* distances = HCSR04.measureDistanceCm(temperature);
		elapsedUs = micros() - startedUs;

		snprintf(line, sizeof(line), "Messung %2d: %8.2f %8.2f %8.2f cm in %6lu us bei %.1f C",
			round, distances[0], distances[1], distances[2], elapsedUs, temperature);
		Serial.println(line);

		if (!checkRound(round, distances, elapsedUs)) return;

		delay(roundPauseMs);
	}

	reportVerdict(elapsedUs, temperature);
}
