#include <HCSR04.h>

int triggerPin = 21;
short echoCount = 2;
int* echoPins = new int[echoCount] { 12, 13 };

void setup () {
  Serial.begin(9600);
  HCSR04.begin(triggerPin, echoPins, echoCount);
}

void loop () {
  unsigned long* times = HCSR04.measureMicroseconds();
  
  for (int i = 0; i < echoCount; i++) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(times[i]);
    Serial.println("μs");
  }
  
  Serial.println("---");
  delay(500);
}