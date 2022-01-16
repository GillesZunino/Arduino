#include <Arduino.h>

#if DEBUG
#include "avr8-stub.h"
#endif


#define DEBOUNCE_DELAY_IN_MS 1


#define BUITLIN_LED_PIN 13
#define DIRECTION_BUTTON_PIN 50
#define SPEED_ANALOG_PIN A0
#define MOTOR_CONTROL_PWM_PIN 3

#define DIRECTION_1A_PIN 2
#define DIRECTION_2A_PIN 7

bool isLeft = true;
uint8_t currentMotorSpeed = 0;


bool readButtonStateWithDebounce(uint8_t pin, int& pinState)
{
  static int currentPinState;
  static int lastPinState = HIGH;

  static unsigned long lastPinChangeTime = 0;

  // Read the pin
  int reading = digitalRead(DIRECTION_BUTTON_PIN);
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

bool getDirection() {
  // Read the  button state and account for switch bounce
  int buttonState;
  if (readButtonStateWithDebounce(DIRECTION_BUTTON_PIN, buttonState))
  {
      // We have a stable reading - The pin is configured with INPUT_PULLUP so it is LOW when pressed and HIGH when released
      if (buttonState == LOW)
      {
        isLeft = !isLeft;
        return true;
      }
  }

  return false;
}

void setDirection(bool left)
{
  if (left)
  {
    // 1A = H, 2A = L -> Turn left
    digitalWrite(DIRECTION_1A_PIN, HIGH);
    digitalWrite(DIRECTION_2A_PIN, LOW);
  }
  else
  {
    // 1A = L, 2A = H -> Turn right
    digitalWrite(DIRECTION_1A_PIN, LOW);
    digitalWrite(DIRECTION_2A_PIN, HIGH);
  }
}

void setup() {
#if DEBUG
  // Initialize GDB stub
  debug_init();
#endif

  pinMode(BUITLIN_LED_PIN, OUTPUT);
  digitalWrite(BUITLIN_LED_PIN, isLeft ? HIGH : LOW);

  pinMode(SPEED_ANALOG_PIN, INPUT);
  pinMode(DIRECTION_BUTTON_PIN, INPUT_PULLUP);

  pinMode(DIRECTION_1A_PIN, OUTPUT);
  pinMode(DIRECTION_2A_PIN, OUTPUT);
  setDirection(isLeft);
}

void loop() {
  bool hasChanged = getDirection();
  if (hasChanged)
  {
    digitalWrite(BUITLIN_LED_PIN, isLeft ? HIGH : LOW);
    setDirection(isLeft);
  }

  // Read speed from potentiometer - It will be betwen 0 and 1023 since the Analog to Digital conversion is 10 bits
  int speedPotentiometerValue = analogRead(SPEED_ANALOG_PIN);
  uint8_t requestedSpeed = map(speedPotentiometerValue, 0, 1023, 0, 255);
  if (currentMotorSpeed != requestedSpeed)
  {
    currentMotorSpeed = requestedSpeed;
    analogWrite(MOTOR_CONTROL_PWM_PIN, requestedSpeed);
  }
}