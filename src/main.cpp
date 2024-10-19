// Ramiro Iván Ríos 2024.

#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hola Mundo!");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1500);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}