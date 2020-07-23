#include <HCSR04.h>

int triggerPin = 21;
int echoPin = 12;

void setup () {
  Serial.begin(9600);
  HCSR04.begin(triggerPin, echoPin);
}

void loop () {
  double* distances = HCSR04.measureDistanceCm();
  
  Serial.print("1: ");
  Serial.print(distances[0]);
  Serial.println("cm");
  
  Serial.println("---");
  delay(500);
}