#include <HCSR04.h>

byte triggerPin = 21;
byte echoCount = 2;
byte* echoPins = new byte[echoCount] { 12, 13 };

void setup () {
  Serial.begin(9600);
  HCSR04.begin(triggerPin, echoPins, echoCount);
}

void loop () {
  long* times = HCSR04.measureMicroseconds();
  
  for (int i = 0; i < echoCount; i++) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(times[i]);
    Serial.println(" μs");
  }
  
  Serial.println("---");
  delay(250);
}