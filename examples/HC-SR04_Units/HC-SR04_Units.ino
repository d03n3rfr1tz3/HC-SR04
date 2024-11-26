#include <HCSR04.h>

// SET THESE VALUES TO MATCH YOUR WIRING -----------
byte triggerPin = 21; //All triggers need to be connected to the same Arduino pin
byte echoCount = 2; // This is the number of sensors that you're using with each echo pin connected to its own Arduino pin
byte* echoPins = new byte[echoCount] { 12, 13 }; //These are the pin numbers of all echo pins
// -------------------------------------------------

long* times = new long[echoCount];
double* distances = new double[echoCount];

void setup () {
  Serial.begin(9600);
  HCSR04.begin(triggerPin, echoPins, echoCount);
}

void loop () {
  HCSR04.measureMicroseconds(times);
  
  for (int i = 0; i < echoCount; i++) {
    if (i > 0) Serial.print(" | ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(times[i]);
    Serial.print(" μs");
  }
  
  Serial.println("");
  delay(100);

  HCSR04.measureDistanceM(distances);

  for (int i = 0; i < echoCount; i++) {
    if (i > 0) Serial.print(" | ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.print(" M");
  }

  Serial.println("");
  delay(100);

  HCSR04.measureDistanceMm(distances);
  
  for (int i = 0; i < echoCount; i++) {
    if (i > 0) Serial.print(" | ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.print(" mm");
  }
  
  Serial.println("");
  delay(100);

  HCSR04.measureDistanceCm(distances);
  
  for (int i = 0; i < echoCount; i++) {
    if (i > 0) Serial.print(" | ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.print(" cm");
  }
  
  Serial.println("");
  delay(100);

  HCSR04.measureDistanceIn(distances);
  
  for (int i = 0; i < echoCount; i++) {
    if (i > 0) Serial.print(" | ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.print(" in");
  }

  Serial.println("");
  delay(100);

  HCSR04.measureDistanceFt(distances);

  for (int i = 0; i < echoCount; i++) {
    if (i > 0) Serial.print(" | ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.print(" ft");
  }

  Serial.println("");
  delay(100);

  HCSR04.measureDistanceYd(distances);

  for (int i = 0; i < echoCount; i++) {
    if (i > 0) Serial.print(" | ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.print(" yd");
  }

  Serial.println("");
  Serial.println("---");
  delay(200);
}
