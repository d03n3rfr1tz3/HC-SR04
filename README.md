[![Compile Sketch](https://github.com/d03n3rfr1tz3/HC-SR04/actions/workflows/compile-sketch.yml/badge.svg)](https://github.com/d03n3rfr1tz3/HC-SR04/actions/workflows/compile-sketch.yml)

# Arduino/ESP8266/ESP32 library for HC-SR04 ultrasonic distance sensor.

This library is used for measuring distance with the HC-SR04, which is an ultrasonic sensor that measures distances from 2 to 400cm. My library features a temperature correction and can utilize one trigger pin with multiple echo pins in parallel. I based this library on two other libraries.
1. https://github.com/Martinsos/arduino-lib-hc-sr04 --> I basically used the temperature calculation
2. https://github.com/gamegine/HCSR04-ultrasonic-sensor-lib --> I basically used the idea of using one trigger and multiple echos. But instead of measuring them in series, I tried to do that in parallel.

![HC-SR04](/HC-SR04.png)

## Usage


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