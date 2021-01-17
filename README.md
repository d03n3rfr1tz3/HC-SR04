# Arduino/ESP8266/ESP32 library for HC-SR04 ultrasonic distance sensor.

This library is used for measuring distance with the HC-SR04, which is an ultrasonic sensor that measures distances from 2 to 400cm. My library features a temperature correction and can utilize one trigger pin with multiple echo pins in parallel. I based this library on two other libraries.
1. https://github.com/Martinsos/arduino-lib-hc-sr04 --> I basically used the temperature calculation
2. https://github.com/gamegine/HCSR04-ultrasonic-sensor-lib --> I basically used the idea of using one trigger and multiple echos. But instead of measuring them in series, I tried to do that in parallel.

![HC-SR04](/HC-SR04.png)

## Usage


## Example

```c
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
