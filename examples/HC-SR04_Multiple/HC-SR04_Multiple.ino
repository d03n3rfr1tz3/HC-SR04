#include <HCSR04.h>

uint8_t triggerPin = 21;
uint8_t echoCount = 2;
uint8_t* echoPins = new uint8_t[echoCount] { 12, 13 };

void setup () {
  Serial.begin(9600);
  HCSR04.begin(triggerPin, echoPins, echoCount);
}

void loop () {
  double* distances = HCSR04.measureDistanceCm();
  
  for (int i = 0; i < echoCount; i++) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.println(" cm");
  }
  
  Serial.println("---");
  delay(500);
}