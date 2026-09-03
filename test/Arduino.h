/*
  Minimal Arduino.h mock for host-side (native) unit tests of the HC-SR04 library.

  It emulates just enough of the Arduino core to compile and run HCSR04.cpp on a
  normal PC with g++/clang. There is no real hardware: a virtual microsecond clock
  drives micros(), delayMicroseconds()/delay() advance it, and every pin carries a
  scripted waveform (a start level plus a list of absolute edge times).

  Advancing the clock replays that waveform microsecond by microsecond. Pins that
  are marked interrupt capable also dispatch to the handler the library attached,
  honouring RISING/FALLING/CHANGE, so both code paths of the library can be driven
  deterministically:

    - interruptCapable == false (default) -> digitalPinToInterrupt() reports
      NOT_AN_INTERRUPT and the library takes its polled path.
    - interruptCapable == true            -> the library attaches interrupts and
      the mock fires them off the scripted waveform.

  pinMode() records every pin it is called with, so tests can assert which pins a
  call actually touched.

  This is a TEST-ONLY header. It is never shipped or compiled into firmware.
*/

#ifndef ARDUINO_MOCK_H
#define ARDUINO_MOCK_H

#include <cstdint>
#include <cstddef>

typedef uint8_t byte;

#define OUTPUT          0x01
#define INPUT           0x00
#define INPUT_PULLUP    0x02

#define LOW             0x00
#define HIGH            0x01

#define RISING          0x01
#define FALLING         0x02
#define CHANGE          0x03

#define NOT_AN_INTERRUPT (-1)

typedef void (*hcsr04_isr_t)(void);

// ---- Mock state ----------------------------------------------------------

namespace hcsr04_mock {
	const int MAX_PINS  = 64;
	const int MAX_EDGES = 8;
	const int MAX_TOUCHED = 64;

	// One scripted echo line: a start level and the absolute times (in virtual
	// microseconds, counted from the last reset) at which the level toggles.
	struct Line {
		int startLevel;
		unsigned long edges[MAX_EDGES];
		int edgeCount;
		int lastLevel;                // level at the previous clock step
		hcsr04_isr_t isr;             // handler the library attached, if any
		int isrMode;                  // RISING / FALLING / CHANGE
	};

	extern unsigned long virtualMicros;
	extern bool interruptCapable;     // what digitalPinToInterrupt() reports
	extern Line lines[MAX_PINS];

	// Pins passed to pinMode() since the last reset, in call order.
	extern uint8_t touched[MAX_TOUCHED];
	extern int touchedCount;

	void reset();

	// A single HIGH pulse on one pin: LOW, rising at startUs, falling widthUs later.
	// A width of 0 means "no pulse at all".
	void schedulePulse(uint8_t pin, unsigned long startUs, unsigned long widthUs);

	// A fully scripted waveform: the level at time 0 plus the toggle times.
	void scheduleEdges(uint8_t pin, int startLevel, const unsigned long* edges, int count);

	int  level(uint8_t pin);              // current level of a pin
	void advance(unsigned long us);       // advance the clock, dispatching interrupts
	bool onlyTouched(const uint8_t* allowed, int allowedCount);
}

// ---- Arduino core surface used by the library ----------------------------

inline unsigned long micros() { return hcsr04_mock::virtualMicros; }
inline unsigned long millis() { return hcsr04_mock::virtualMicros / 1000UL; }

inline void delayMicroseconds(unsigned int us) { hcsr04_mock::advance(us); }
inline void delay(unsigned long ms)            { hcsr04_mock::advance(ms * 1000UL); }

inline void pinMode(uint8_t pin, uint8_t) {
	using namespace hcsr04_mock;
	if (touchedCount < MAX_TOUCHED) touched[touchedCount++] = pin;
}

inline void digitalWrite(uint8_t, uint8_t) {}

inline int digitalRead(uint8_t pin) { return hcsr04_mock::level(pin); }

inline int digitalPinToInterrupt(uint8_t pin) {
	// Interrupt numbers are pin numbers here, which keeps the mapping obvious.
	return hcsr04_mock::interruptCapable ? (int) pin : NOT_AN_INTERRUPT;
}

inline void attachInterrupt(int interrupt, hcsr04_isr_t fn, int mode) {
	using namespace hcsr04_mock;
	if (interrupt < 0 || interrupt >= MAX_PINS) return;
	lines[interrupt].isr = fn;
	lines[interrupt].isrMode = mode;
}

inline void detachInterrupt(int interrupt) {
	using namespace hcsr04_mock;
	if (interrupt < 0 || interrupt >= MAX_PINS) return;
	lines[interrupt].isr = NULL;
}

#endif // ARDUINO_MOCK_H
