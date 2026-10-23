[![Compile Sketch](https://github.com/d03n3rfr1tz3/HC-SR04/actions/workflows/compile-sketch.yml/badge.svg)](https://github.com/d03n3rfr1tz3/HC-SR04/actions/workflows/compile-sketch.yml) [![Native Tests](https://github.com/d03n3rfr1tz3/HC-SR04/actions/workflows/native-test.yml/badge.svg)](https://github.com/d03n3rfr1tz3/HC-SR04/actions/workflows/native-test.yml)

# Arduino/ESP8266/ESP32 library for HC-SR04 ultrasonic distance sensor.

This library is used for measuring distance with the HC-SR04, which is an ultrasonic sensor that measures distances from 2 to 400cm. My library features a temperature correction and can utilize one trigger pin with multiple echo pins in parallel. I based this library on two other libraries.
1. https://github.com/Martinsos/arduino-lib-hc-sr04 --> I basically used the temperature calculation
2. https://github.com/gamegine/HCSR04-ultrasonic-sensor-lib --> I basically used the idea of using one trigger and multiple echos. But instead of measuring them in series, I tried to do that in parallel.

![HC-SR04](/HC-SR04.png)

## Usage

The library ships a ready-made instance called `HCSR04`, so you do not create an object yourself. Call `begin()` once in `setup()` and then call one of the measure methods whenever you want a reading.

```c++
HCSR04.begin(triggerPin, echoPins, echoCount);
```

The full form of `begin()` takes four more arguments, all of which have a default.

| Argument      | Default      | Meaning |
|---------------|--------------|---------|
| `timeout`     | `100000`     | How long to wait for an echo, in microseconds. |
| `triggerTime` | `10`         | Length of the trigger pulse in microseconds. Some sensors need a longer one. |
| `triggerWait` | `10`         | Pause after the trigger pulse in microseconds, so the pulse stays out of the measurement. |
| `unlock`      | `unlockSkip` | What to do about a sensor that holds its echo line, see below. |

Echo pins that the board can use as an interrupt are measured with an interrupt handler, all other pins are polled. The library picks the path per pin, there is nothing to configure. It supports up to `HCSR04_MAX_SENSORS` echo pins, currently ten, because it provides one interrupt handler per sensor. Further pins passed to `begin()` are ignored.

Every measure method comes in two shapes. Without an output array it returns a pointer to a buffer inside the library, which is valid until the next measurement. With an output array it writes the results there and the array stays yours.

### Error values

A measurement that has nothing to report writes a negative value. The three cases stay distinguishable all the way through the distance methods, and a check for `< 0` catches all of them.

| Constant                | Value | Meaning |
|-------------------------|-------|---------|
| `HCSR04_INVALID_RESULT` | `-1`  | A duration was measured, but the distance lies outside the supported range. |
| `HCSR04_NO_TRIGGER`     | `-2`  | The echo line never went HIGH. Usually bad wiring or a sensor without power. |
| `HCSR04_NO_ECHO`        | `-3`  | The echo started but did not end before the timeout. Usually nothing in range. |

### Unlocking a stuck sensor

An HC-SR04 can end up holding its echo line HIGH forever, for example after a reset in the middle of a measurement. It then never reports anything again. `begin()` can pull the echo pins LOW for a moment to get the sensor out of that state.

| Value          | Behaviour |
|----------------|-----------|
| `unlockSkip`   | Do nothing. This is the default. |
| `unlockMaybe`  | Unlock only those pins that are HIGH at the time of the call. |
| `unlockForced` | Always unlock every echo pin. |

An unlock costs about 200 ms, so `unlockForced` makes every `begin()` that much slower.

```c++
HCSR04.begin(triggerPin, echoPins, echoCount, 100000, HCSR04Sensor::unlockMaybe);
```


## Example

```c++
#include <HCSR04.h>

byte triggerPin = 21;
byte echoCount = 2;
byte* echoPins = new byte[echoCount] { 12, 13 };

void setup () {
  Serial.begin(115200);
  HCSR04.begin(triggerPin, echoPins, echoCount);
}

void loop () {
  double* distances = HCSR04.measureDistanceCm();
  
  for (int i = 0; i < echoCount; i++) {
    if (i > 0) Serial.print(" | ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.print(" cm");
  }
  
  Serial.println("");
  delay(500);
}
```

## Library Options

You can get the distance back in several different units, using these methods:

| Method                | Unit         |
|-----------------------|--------------|
| `measureDistanceM()`  | `Meters`     |
| `measureDistanceMm()` | `Millimeters`|
| `measureDistanceCm()` | `Centimeters`|
| `measureDistanceIn()` | `Inches`     |
| `measureDistanceFt()` | `Feet`       |
| `measureDistanceYd()` | `Yards`      |

You can optionally pass into the method, the current ambient temperature in degrees Celcius for better accuracy.

### Examples
```c++
float tempC = 22.2222222 // 72 Degrees F
float distance = measureDistanceCm(tempC)
```

```c++
#include <HCSR04.h>

byte triggerPin = 21;
byte echoCount = 2;
byte* echoPins = new byte[echoCount] { 12, 13 };
const double tempC = 22.2222; //72 degrees F
double* distances = new double[echoCount];

void setup () {
  Serial.begin(115200);
  HCSR04.begin(triggerPin, echoPins, echoCount);
}

void loop () {
  HCSR04.measureDistanceMm(tempC, distances);
  
  for (int i = 0; i < echoCount; i++) {
    if (i > 0) Serial.print(" | ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.print(" mm");
  }
  
  Serial.println("");
  delay(500);
}
```

## Simulation

The examples can be tried out in the browser without any hardware. The matching Wokwi
diagrams and a short guide are in [extras/wokwi](extras/wokwi).
