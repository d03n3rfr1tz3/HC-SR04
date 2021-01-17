#include <HCSR04.h>

byte triggerPin = 21;
byte echoPin = 12;

void setup () {
  Serial.begin(9600);
  HCSR04.begin(triggerPin, echoPin);
}

void loop () {
  float temperature = 15;
  
  double* distances = HCSR04.measureDistanceCm(temperature);
  
  Serial.print("1: ");
  Serial.print(distances[0]);
  Serial.println(" cm");
  
  Serial.println("---");
  delay(250);
}