/* Storage and helpers backing the Arduino.h test mock. TEST-ONLY. */

#include "Arduino.h"

namespace hcsr04_mock {
	unsigned long virtualMicros = 0;
	bool interruptCapable = false;
	Line lines[MAX_PINS];

	uint8_t touched[MAX_TOUCHED];
	int touchedCount = 0;

	void reset() {
		virtualMicros = 0;
		interruptCapable = false;
		touchedCount = 0;
		for (int i = 0; i < MAX_PINS; i++) {
			lines[i].startLevel = LOW;
			lines[i].edgeCount = 0;
			lines[i].lastLevel = LOW;
			lines[i].isr = NULL;
			lines[i].isrMode = 0;
		}
	}

	int level(uint8_t pin) {
		if (pin >= MAX_PINS) return LOW;
		const Line& l = lines[pin];
		int lv = l.startLevel;
		for (int k = 0; k < l.edgeCount; k++) {
			if (virtualMicros >= l.edges[k]) lv = (lv == HIGH) ? LOW : HIGH;
			else break;
		}
		return lv;
	}

	void scheduleEdges(uint8_t pin, int startLevel, const unsigned long* edges, int count) {
		if (pin >= MAX_PINS) return;
		Line& l = lines[pin];
		l.startLevel = startLevel;
		l.edgeCount = count > MAX_EDGES ? MAX_EDGES : count;
		for (int k = 0; k < l.edgeCount; k++) l.edges[k] = edges[k];
		l.lastLevel = level(pin);
	}

	void schedulePulse(uint8_t pin, unsigned long startUs, unsigned long widthUs) {
		if (widthUs == 0) {
			scheduleEdges(pin, LOW, NULL, 0);
		} else {
			unsigned long edges[2] = { startUs, startUs + widthUs };
			scheduleEdges(pin, LOW, edges, 2);
		}
	}

	// Step the clock one microsecond at a time so no scripted edge is skipped, and
	// dispatch to whatever handler the library has attached to that pin.
	void advance(unsigned long us) {
		for (unsigned long step = 0; step < us; step++) {
			virtualMicros++;
			for (int pin = 0; pin < MAX_PINS; pin++) {
				Line& l = lines[pin];
				int now = level((uint8_t) pin);
				if (now == l.lastLevel) continue;

				bool rising = (now == HIGH);
				l.lastLevel = now;
				if (l.isr == NULL) continue;
				if (l.isrMode == CHANGE ||
				    (l.isrMode == RISING && rising) ||
				    (l.isrMode == FALLING && !rising)) {
					l.isr();
				}
			}
		}
	}

	bool onlyTouched(const uint8_t* allowed, int allowedCount) {
		for (int i = 0; i < touchedCount; i++) {
			bool ok = false;
			for (int k = 0; k < allowedCount; k++) if (touched[i] == allowed[k]) { ok = true; break; }
			if (!ok) return false;
		}
		return true;
	}
}
