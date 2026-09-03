/*
  Host-side unit tests for the HC-SR04 library.

  These run on a normal PC (no hardware) against the mock Arduino.h in this folder.
  They cover the parts that are deterministic and worth guarding against regressions:
    - the microsecond -> distance math and unit conversions
    - cross-unit consistency (e.g. meters * 100 == centimeters)
    - the error sentinels for "no trigger" / out-of-range
    - the interrupt path, including an echo line that is still busy when the
      interrupt is armed
    - a bounds regression guard for unlockSensors()

  Real timing accuracy still needs hardware; that is out of scope here by design.
  What the mock does reproduce faithfully is the edge/level *logic* of both paths.
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
// Returns the measured distance via the supplied member function pointer.
static double measureWithPulse(MeasureFn fn, unsigned long widthUs, float tempC) {
	hcsr04_mock::reset();
	// The library emits a ~24us trigger pulse before listening; start the echo
	// comfortably after that so the rising edge is seen cleanly.
	hcsr04_mock::schedulePulse(ECHO, 100, widthUs);

	HCSR04.begin(TRIGGER, ECHO);

	double out = 0.0;
	(HCSR04.*fn)(tempC, &out);
	return out;
}

// Drive one measurement over the interrupt path with a fully scripted waveform.
static long measureUsWithEdges(int startLevel, const unsigned long* edges, int edgeCount) {
	hcsr04_mock::reset();
	hcsr04_mock::interruptCapable = true;
	hcsr04_mock::scheduleEdges(ECHO, startLevel, edges, edgeCount);

	HCSR04.begin(TRIGGER, ECHO);

	long us = 0;
	HCSR04.measureMicroseconds(&us);
	return us;
}

int main() {
	const float T = 20.0f; // degrees Celsius
	const double speed = speedCmPerUs(T);

	printf("Distance math\n");
	{
		// 100 cm target => round-trip time = 2 * distance / speed.
		unsigned long width = (unsigned long) std::lround(2.0 * 100.0 / speed);
		double cm = measureWithPulse(&HCSR04Sensor::measureDistanceCm, width, T);
		// Loop granularity is ~1us, i.e. well under 0.1 cm; allow 0.5 cm slack.
		checkNear(cm, 100.0, 0.5, "measureDistanceCm ~100 cm");
	}
	{
		unsigned long width = (unsigned long) std::lround(2.0 * 10.0 / speed);
		double cm = measureWithPulse(&HCSR04Sensor::measureDistanceCm, width, T);
		checkNear(cm, 10.0, 0.3, "measureDistanceCm ~10 cm");
	}

	printf("Unit consistency (regression guard for the 331.3 vs 331.4 fix)\n");
	{
		unsigned long width = (unsigned long) std::lround(2.0 * 50.0 / speed);
		double m  = measureWithPulse(&HCSR04Sensor::measureDistanceM,  width, T);
		double cm = measureWithPulse(&HCSR04Sensor::measureDistanceCm, width, T);
		double mm = measureWithPulse(&HCSR04Sensor::measureDistanceMm, width, T);
		// Same scripted pulse => identical duration => units must line up exactly
		// (they only do if M and Cm share the same speed-of-sound constant).
		checkNear(m * 100.0, cm, 1e-6, "meters * 100 == centimeters");
		checkNear(cm * 10.0, mm, 1e-6, "centimeters * 10 == millimeters");
	}

	printf("Error sentinels\n");
	{
		// No pulse at all => the sensor never sees an echo start => NO_TRIGGER (-2),
		// which the distance layer collapses to the invalid sentinel (< 0).
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
	}

	printf("Interrupt path\n");
	{
		// Healthy sensor: line idle LOW, echo rises at 450us.
		unsigned long width = (unsigned long) std::lround(2.0 * 100.0 / speed);
		unsigned long edges[2] = { 450, 450 + width };
		long us = measureUsWithEdges(LOW, edges, 2);
		checkNear(us / 2.0 * speed, 100.0, 0.5, "interrupt echo ~100 cm");
	}
	{
		// A close object produces a very short echo (~116us at 2 cm).
		// Note this does NOT reproduce the attach/detach race of the real
		// implementation: attaching and detaching are instantaneous against the
		// virtual clock, so no edge can slip through the gap here. Only real
		// hardware can show that. What this does cover is that a short echo is
		// measured correctly end to end.
		unsigned long width = (unsigned long) std::lround(2.0 * 2.0 / speed);
		unsigned long edges[2] = { 450, 450 + width };
		long us = measureUsWithEdges(LOW, edges, 2);
		checkNear(us / 2.0 * speed, 2.0, 0.3, "interrupt echo ~2 cm (short pulse)");
	}
	{
		// The echo line is still HIGH from a previous cycle when the interrupt is
		// armed and only drops at 300us; the echo for *this* measurement runs
		// 450 .. 450+width. The RISING handler correctly ignores that first falling
		// edge and latches on the real rising edge.
		//
		// Keep this guard when the single-CHANGE-interrupt rework lands: a handler
		// that merely counts edges takes the falling edge at 300us for the start of
		// the measurement and reports a plausible looking ~2.5 cm instead of 100 cm.
		unsigned long width = (unsigned long) std::lround(2.0 * 100.0 / speed);
		unsigned long edges[3] = { 300, 450, 450 + width };
		long us = measureUsWithEdges(HIGH, edges, 3);
		checkNear(us / 2.0 * speed, 100.0, 0.5, "busy echo line at arm time does not fake a near object");
	}

	printf("Bounds guard: unlockSensors stays inside echoPins\n");
	{
		// With unlockForced the unlock loop runs. The old code walked echoPins as if
		// it were null-terminated and read past the end, driving whatever pin numbers
		// happened to follow in memory. The trailing bytes here are deliberately
		// non-zero so that misbehaviour is caught even without a sanitizer; under
		// -fsanitize=address the pre-fix version additionally trips a
		// heap-buffer-overflow on this allocation.
		uint8_t* echoPins = new uint8_t[8]{ 12, 13, 7, 4, 22, 9, 3, 6 };

		hcsr04_mock::reset();
		HCSR04.begin(TRIGGER, echoPins, 2, 100000, HCSR04Sensor::unlockForced);

		const uint8_t allowed[3] = { TRIGGER, 12, 13 };
		check(hcsr04_mock::onlyTouched(allowed, 3), "begin() with unlockForced touches only the configured pins");
		delete[] echoPins;
	}

	printf("\n%d checks, %d failure(s)\n", checks, failures);
	return failures == 0 ? 0 : 1;
}
