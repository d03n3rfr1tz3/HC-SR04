#include <HCSR04.h>

int triggerPin = 21;
int* echoPins = new int[2] { 12, 13 };

void setup () {
  Serial.begin(9600);
  HCSR04.begin(triggerPin, echoPins);
}

void loop () {
  size_t size = sizeof(echoPins)/sizeof(echoPins[0]);
  double* distances = HCSR04.measureDistanceCm();
  
  for (int i = 0; i < size; i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.println("---");
  }
  
  delay(500);
}
