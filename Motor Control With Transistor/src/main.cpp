#include <Arduino.h>

#if DEBUG
#include "avr8-stub.h"
#endif


#define DEBOUNCE_DELAY_IN_MS 1


#define BUITLIN_LED_PIN 13
#define ON_OFF_BUTTON_PIN 50
#define SPEED_ANALOG_PIN A0
#define MOTOR_CONTROL_PWM_PIN 3


bool isMotorOn = false;
uint8_t currentMotorSpeed = 0;


bool readButtonStateWithDebounce(uint8_t pin, int& pinState)
{
  static int currentPinState;
  static int lastPinState = HIGH;

  static unsigned long lastPinChangeTime = 0;

  // Read the pin
  int reading = digitalRead(ON_OFF_BUTTON_PIN);
  if (reading != lastPinState)
  {
    // Bounce detected - Reset the last pin changed time
    lastPinChangeTime = millis();
  }

  // Remember the last pin state
  lastPinState = reading;

  if ((millis() - lastPinChangeTime) > DEBOUNCE_DELAY_IN_MS)
  {
    // The state has been the same for longer than the debounce delays so consider it stable
    if (reading != currentPinState)
    {
      pinState = currentPinState = reading;
      return true;
    }
  }
  return false;
}

bool getOnOff() {
  // Read the  button state and account for switch bounce
  int buttonState;
  if (readButtonStateWithDebounce(ON_OFF_BUTTON_PIN, buttonState))
  {
      // We have a stable reading - The pin is configured with INPUT_PULLUP so it is LOW when pressed and HIGH when released
      if (buttonState == LOW)
      {
        isMotorOn = !isMotorOn;
        return true;
      }
  }

  return false;
}

void setup() {
#if DEBUG
  // Initialize GDB stub
  debug_init();
#endif

  pinMode(BUITLIN_LED_PIN, OUTPUT);
  digitalWrite(BUITLIN_LED_PIN, LOW);

  pinMode(SPEED_ANALOG_PIN, INPUT);
  pinMode(ON_OFF_BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  bool hasChanged = getOnOff();
  if (hasChanged)
  {
    digitalWrite(BUITLIN_LED_PIN, isMotorOn ? HIGH : LOW);
    if (!isMotorOn)
    {
        currentMotorSpeed = 0;
        analogWrite(MOTOR_CONTROL_PWM_PIN, LOW);
    }
  }

  if (isMotorOn)
  {
    // Read speed from potentiometer - It will be betwen 0 and 1023 since the Analog to Digital conversion is 10 bits
    int speedPotentiometerValue = analogRead(SPEED_ANALOG_PIN);
    uint8_t requestedSpeed = map(speedPotentiometerValue, 0, 1023, 0, 255);
    if (currentMotorSpeed != requestedSpeed)
    {
      currentMotorSpeed = requestedSpeed;
      analogWrite(MOTOR_CONTROL_PWM_PIN, requestedSpeed);
    } 
  }
}