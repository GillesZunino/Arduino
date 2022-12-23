#include <Arduino.h>

#include <avr8-stubs-utilities.h>


// Use the builti in LED to show initialization and movement
#define MOVEMENT_MONITOR_LED_PIN LED_BUILTIN

//
// PIR sensor pinout (potentiometers facing up)
//      Power - Output - Ground
//
// Connect 'Output' to a Digital pin
//
#define PIR_SENSOR_DATA_PIN 8


unsigned long startTimeMs = 0;

void setup() {
  INIT_DEBUGGER();

  // The PIR Sensor takes about 1 minute to initialize - Remember the startup time
  startTimeMs = millis();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIR_SENSOR_DATA_PIN, INPUT);
  pinMode(MOVEMENT_MONITOR_LED_PIN, OUTPUT);
}

void loop() {
  // If we are within the PIR sensor initialization time (about 1 minute), blink the on board LED
  unsigned long currentTimeMs = millis();
  if (currentTimeMs - startTimeMs < (60UL * 1000UL)) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
  } else {
    // Sensor is initialized - Read data and have the LED floow readings
    int pirData = digitalRead(PIR_SENSOR_DATA_PIN);
    digitalWrite(MOVEMENT_MONITOR_LED_PIN, pirData > 0 ? HIGH : LOW);
  }
}