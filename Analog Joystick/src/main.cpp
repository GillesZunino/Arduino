#include <Arduino.h>

#if DEBUG
#include "avr8-stub.h"
#include "app_api.h"
#endif


#define X_AXIS_ANALOG_PIN A1
#define Y_AXIS_ANALOG_PIN A2

#define PUSH_SWITCH_PIN 9


void setup() {
#if DEBUG
  // Initialize GDB stub - The debugger uses INT0 [pin 2 (Uno) or pin 21 (Mega)] so DO NOT USE these pins !
  debug_init();
#endif

  pinMode(X_AXIS_ANALOG_PIN, INPUT);
  pinMode(Y_AXIS_ANALOG_PIN, INPUT);
  pinMode(PUSH_SWITCH_PIN, INPUT_PULLUP);

  Serial.begin(115200);
}

void loop() {
  int xPos = analogRead(X_AXIS_ANALOG_PIN);
  int yPos = analogRead(Y_AXIS_ANALOG_PIN);
  bool switchPressed = digitalRead(PUSH_SWITCH_PIN) == LOW;

  String s;
  s = "S:" + String(switchPressed ? "ON" : "OFF") + " X:" + String(xPos) + " Y:" + String(yPos) + "\n";
  Serial.write(s.c_str());
}