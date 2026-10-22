/*
  Host-side unit tests for the HC-SR04 library.

  They run on a normal PC against the mock Arduino.h in this folder and cover the
  deterministic behaviour of both measurement paths. Real timing accuracy needs
  hardware and is out of scope here.
*/

#include <cstdio>
#include <cmath>
#include <cstdint>
#include "Arduino.h"
#include "../src/HCSR04.h"

static int failures = 0;
static int checks = 0;

static void check(bool cond, const char* name) {
	checks++;
	if (!cond) { failures++; printf("  [FAIL] %s\n", name); }
	else       { printf("  [ ok ] %s\n", name); }
}

static void checkNear(double actual, double expected, double tol, const char* name) {
	checks++;
	if (std::fabs(actual - expected) > tol) {
		failures++;
		printf("  [FAIL] %s (expected %.5f, got %.5f, tol %.5f)\n", name, expected, actual, tol);
	} else {
		printf("  [ ok ] %s (%.5f)\n", name, actual);
	}
}

// Speed of sound the library uses, in cm/us, for a given temperature.
static double speedCmPerUs(double tempC) { return ((331.3 + 0.606 * tempC) * 100.0) / 1000000.0; }

// Concrete signature of the measureDistance*(float, double*) overloads.
typedef void (HCSR04Sensor::*MeasureFn)(float, double*);

static const uint8_t TRIGGER = 5;
static const uint8_t ECHO    = 12;

// Drive one measurement for a scripted echo of the given round-trip width.
static double measureWithPulse(MeasureFn fn, unsigned long widthUs, float tempC) {
	hcsr04_mock::reset();
	// The library emits a ~24us trigger pulse before it listens, so the echo starts later.
	hcsr04_mock::schedulePulse(ECHO, 100, widthUs);

	HCSR04.begin(TRIGGER, ECHO);

	double out = 0.0;
	(HCSR04.*fn)(tempC, &out);
	return out;
}

// Drive one measurement with a scripted waveform over the interrupt or the polled path.
static long measureUsWithEdges(bool useInterrupt, int startLevel, const unsigned long* edges,
                               int edgeCount, uint32_t timeout = 100000) {
	hcsr04_mock::reset();
	hcsr04_mock::interruptCapable = useInterrupt;
	hcsr04_mock::scheduleEdges(ECHO, startLevel, edges, edgeCount);

	HCSR04.begin(TRIGGER, ECHO, timeout, HCSR04Sensor::unlockSkip);

	long us = 0;
	HCSR04.measureMicroseconds(&us);
	return us;
}

int main() {
	const float T = 20.0f; // degrees Celsius
	const double speed = speedCmPerUs(T);

	printf("Distance math\n");
	{
		// A 100 cm target means a round-trip time of 2 * distance / speed.
		unsigned long width = (unsigned long) std::lround(2.0 * 100.0 / speed);
		double cm = measureWithPulse(&HCSR04Sensor::measureDistanceCm, width, T);
		// Loop granularity is about 1us, so 0.5 cm slack is plenty.
		checkNear(cm, 100.0, 0.5, "measureDistanceCm ~100 cm");
	}
	{
		unsigned long width = (unsigned long) std::lround(2.0 * 10.0 / speed);
		double cm = measureWithPulse(&HCSR04Sensor::measureDistanceCm, width, T);
		checkNear(cm, 10.0, 0.3, "measureDistanceCm ~10 cm");
	}

	printf("Unit consistency\n");
	{
		// The same scripted pulse has to convert to matching values in every unit.
		unsigned long width = (unsigned long) std::lround(2.0 * 50.0 / speed);
		double m  = measureWithPulse(&HCSR04Sensor::measureDistanceM,  width, T);
		double cm = measureWithPulse(&HCSR04Sensor::measureDistanceCm, width, T);
		double mm = measureWithPulse(&HCSR04Sensor::measureDistanceMm, width, T);
		checkNear(m * 100.0, cm, 1e-6, "meters * 100 == centimeters");
		checkNear(cm * 10.0, mm, 1e-6, "centimeters * 10 == millimeters");
	}

	printf("Error sentinels\n");
	{
		// Without any pulse the sensor never sees an echo start and reports NO_TRIGGER.
		hcsr04_mock::reset();
		hcsr04_mock::schedulePulse(ECHO, 0, 0); // no pulse
		HCSR04.begin(TRIGGER, ECHO, 2000 /* short timeout us */, HCSR04Sensor::unlockSkip);
		long us = 0;
		HCSR04.measureMicroseconds(&us);
		check(us < 0, "measureMicroseconds returns negative sentinel when no echo");

		double cm = 0.0;
		hcsr04_mock::reset();
		HCSR04.measureDistanceCm(&cm);
		check(cm < 0.0, "measureDistanceCm returns invalid (< 0) when no echo");
		check(cm == HCSR04_NO_TRIGGER, "measureDistanceCm keeps the NO_TRIGGER sentinel");
	}
	{
		// The echo starts but never ends, which has to stay distinguishable from NO_TRIGGER.
		unsigned long risesOnly[1] = { 450 };
		hcsr04_mock::reset();
		hcsr04_mock::interruptCapable = true;
		hcsr04_mock::scheduleEdges(ECHO, LOW, risesOnly, 1);
		HCSR04.begin(TRIGGER, ECHO, 2000, HCSR04Sensor::unlockSkip);

		double cm = 0.0;
		HCSR04.measureDistanceCm(&cm);
		check(cm == HCSR04_NO_ECHO, "measureDistanceCm keeps the NO_ECHO sentinel");
	}

	printf("Interrupt path\n");
	{
		// Healthy sensor with an idle LOW line and an echo rising at 450us.
		unsigned long width = (unsigned long) std::lround(2.0 * 100.0 / speed);
		unsigned long edges[2] = { 450, 450 + width };
		long us = measureUsWithEdges(true, LOW, edges, 2);
		checkNear(us / 2.0 * speed, 100.0, 0.5, "interrupt echo ~100 cm");
	}
	{
		// A close object produces a very short echo of about 116us at 2 cm.
		unsigned long width = (unsigned long) std::lround(2.0 * 2.0 / speed);
		unsigned long edges[2] = { 450, 450 + width };
		long us = measureUsWithEdges(true, LOW, edges, 2);
		checkNear(us / 2.0 * speed, 2.0, 0.3, "interrupt echo ~2 cm (short pulse)");
	}

	printf("Busy echo line at start of measurement (both paths)\n");
	{
		// The line still carries the echo of the previous cycle and drops at 300us.
		// Taking that stale HIGH as a start would report a plausible near value instead
		// of the real 100 cm echo between 450us and 450+width.
		unsigned long width = (unsigned long) std::lround(2.0 * 100.0 / speed);
		unsigned long edges[3] = { 300, 450, 450 + width };

		long usInt = measureUsWithEdges(true, HIGH, edges, 3);
		checkNear(usInt / 2.0 * speed, 100.0, 0.5, "interrupt path: busy line does not fake a near object");

		long usPolled = measureUsWithEdges(false, HIGH, edges, 3);
		checkNear(usPolled / 2.0 * speed, 100.0, 0.5, "polled path: busy line does not fake a near object");
	}
	{
		// The line is stuck HIGH without any edge, so both paths have to run into the
		// timeout instead of inventing a duration from the stale level.
		long usInt = measureUsWithEdges(true, HIGH, NULL, 0, 2000);
		check(usInt == HCSR04_NO_TRIGGER, "interrupt path: line stuck HIGH reports NO_TRIGGER");

		long usPolled = measureUsWithEdges(false, HIGH, NULL, 0, 2000);
		check(usPolled == HCSR04_NO_TRIGGER, "polled path: line stuck HIGH reports NO_TRIGGER");
	}

	printf("Interrupt cleanup\n");
	{
		// The first sensor sees a rising edge that never falls and stays inside its own
		// timeout window, while the second one makes the wait loop give up. A handler
		// surviving that would write into the timings of the next measurement.
		const uint8_t ECHO_B = 13;
		uint8_t echoPins[2] = { ECHO, ECHO_B };
		unsigned long risesOnly[1] = { 450 };

		hcsr04_mock::reset();
		hcsr04_mock::interruptCapable = true;
		hcsr04_mock::scheduleEdges(ECHO, LOW, risesOnly, 1); // rises, never falls
		hcsr04_mock::scheduleEdges(ECHO_B, LOW, NULL, 0);    // stays LOW throughout

		HCSR04.begin(TRIGGER, echoPins, 2, 2000, HCSR04Sensor::unlockSkip);
		long us[2] = { 0, 0 };
		HCSR04.measureMicroseconds(us);

		check(hcsr04_mock::lines[ECHO].isr == NULL && hcsr04_mock::lines[ECHO_B].isr == NULL,
		      "no interrupt handler stays attached after a measurement");
	}

	printf("Bounds guard: unlockSensors stays inside echoPins\n");
	{
		// With unlockForced the unlock loop runs. The trailing bytes are deliberately
		// non-zero, so reading past the end would drive pins outside the configured set.
		uint8_t* echoPins = new uint8_t[8]{ 12, 13, 7, 4, 22, 9, 3, 6 };

		hcsr04_mock::reset();
		HCSR04.begin(TRIGGER, echoPins, 2, 100000, HCSR04Sensor::unlockForced);

		const uint8_t allowed[3] = { TRIGGER, 12, 13 };
		check(hcsr04_mock::onlyTouched(allowed, 3), "begin() with unlockForced touches only the configured pins");
		delete[] echoPins;
	}

	printf("Hard limit on the number of sensors\n");
	{
		// Two echo pins more than the library supports. They are dropped in begin(),
		// so the measurement never writes a result for them.
		uint8_t echoPins[12] = { 2, 3, 4, 6, 7, 8, 9, 10, 11, 13, 14, 15 };

		hcsr04_mock::reset();
		hcsr04_mock::interruptCapable = true;
		HCSR04.begin(TRIGGER, echoPins, 12, 2000, HCSR04Sensor::unlockSkip);

		long us[12];
		for (int i = 0; i < 12; i++) us[i] = 1234;
		HCSR04.measureMicroseconds(us);

		check(us[HCSR04_MAX_SENSORS] == 1234 && us[11] == 1234,
		      "begin() caps the sensor count at HCSR04_MAX_SENSORS");

		const uint8_t allowed[11] = { TRIGGER, 2, 3, 4, 6, 7, 8, 9, 10, 11, 13 };
		check(hcsr04_mock::onlyTouched(allowed, 11), "begin() leaves the pins beyond the cap alone");
	}

	// Has to stay last, because HCSR04 is a singleton and this releases its buffers.
	printf("end() releases the sensors\n");
	{
		hcsr04_mock::reset();
		HCSR04.begin(TRIGGER, ECHO);
		HCSR04.end();

		double cm = 42.0;
		HCSR04.measureDistanceCm(&cm);
		check(cm == 42.0, "measureDistanceCm after end() returns without touching the result");
	}

	printf("\n%d checks, %d failure(s)\n", checks, failures);
	return failures == 0 ? 0 : 1;
}
