#include <HCSR04.h>

int triggerPin = 21;
short echoCount = 2;
int* echoPins = new int[echoCount] { 12, 13 };

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
    Serial.println("cm");
  }
  
  Serial.println("---");
  delay(500);
}