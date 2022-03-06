#include <Arduino.h>

#if DEBUG
#include "avr8-stub.h"
#endif


#define DEBOUNCE_DELAY_IN_MS 10


#define BUITLIN_LED_PIN 13
#define DIRECTION_BUTTON_PIN 50
#define SPEED_ANALOG_PIN A0
#define MOTOR_CONTROL_PWM_PIN 3

//
// Direction pins - Optionally, LEDs can be attached to see motor direction
// * - Attach red LED in serie with a 1K Ohm resistor (Vf = 1.85V)
// * - Attach green LED in serie with a 1K Ohm resistor (Vf = 2.8V)
// * - Attach blue LED in serie with a 1K Ohm resistor (Vf = 2.5V)
//
// 1A Digital is "LEFT" - 2A DIGITAL is "RIGHT"
//
#define DIRECTION_1A_PIN 2
#define DIRECTION_2A_PIN 7


bool isLeft;
long currentMotorSpeed = 0;


bool readButtonStateWithDebounce(uint8_t pin, int& pinState)
{
  static int currentPinState = LOW;
  static int lastPinState = LOW;

  static unsigned long lastPinChangeTime = millis();

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

  pinState = reading;
  return false;
}

bool getDirectionChanged() {
  // Read the  button state and account for switch bounce - readButtonStateWithDebounce() will return true if this is a 'legitimate' button state change
  // Our left / right button is configured on a INPUT_PULLUP pin so it will be 'pressed' with its stable state is LOW
  int buttonState;
  return readButtonStateWithDebounce(DIRECTION_BUTTON_PIN, buttonState) && (buttonState == LOW);
}

void setDirection(bool left)
{
  isLeft = left;

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

  // Configure BUITLIN_LED_PIN and turn the built in LED on to indicate we are initializing
  pinMode(BUITLIN_LED_PIN, OUTPUT);
  digitalWrite(BUITLIN_LED_PIN, HIGH);

  pinMode(SPEED_ANALOG_PIN, INPUT);
  pinMode(DIRECTION_BUTTON_PIN, INPUT_PULLUP);

  pinMode(DIRECTION_1A_PIN, OUTPUT);
  pinMode(DIRECTION_2A_PIN, OUTPUT);

  // Start with the motor going 'left'
  setDirection(true);

  // Turn the built in LED off to indicate we are ready
  digitalWrite(BUITLIN_LED_PIN, LOW);
}

void loop() {
  if (getDirectionChanged())
  {
    setDirection(!isLeft);
  }

  // Read speed from potentiometer - It will be betwen 0 and 1023 since the Analog to Digital conversion is 10 bits
  // Consider a reading or 0 on the potentiometer as "stop" and anything above 0 as the minimum PWN
  int speedPotentiometerValue = analogRead(SPEED_ANALOG_PIN);
  long requestedSpeed = map(speedPotentiometerValue, 0, 1023, 0, 255);

  if (currentMotorSpeed != requestedSpeed)
  {
    currentMotorSpeed = requestedSpeed;
    analogWrite(MOTOR_CONTROL_PWM_PIN, requestedSpeed);
  }
}