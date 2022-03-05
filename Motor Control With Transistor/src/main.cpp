#include <Arduino.h>

#if DEBUG
#include "avr8-stub.h"
#endif


#define DEBOUNCE_DELAY_IN_MS 10


#define BUITLIN_LED_PIN 13
#define ON_OFF_BUTTON_PIN 50
#define SPEED_ANALOG_PIN A0
#define MOTOR_CONTROL_PWM_PIN 3


bool isMotorOn;
long currentMotorSpeed;


bool readButtonStateWithDebounce(uint8_t pin, int& pinState)
{
  static int currentPinState = LOW;
  static int lastPinState = LOW;

  static unsigned long lastPinChangeTime = millis();

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

  pinState = reading;
  return false;
}

bool getOnOffPressed()
{
  // Read the  button state and account for switch bounce - readButtonStateWithDebounce() will return true if this is a 'legitimate' button state change
  // Our on / off button is configured on a INPUT_PULLUP pin so it will be 'pressed' with its stable state is LOW
  int buttonState;
  return readButtonStateWithDebounce(ON_OFF_BUTTON_PIN, buttonState) && (buttonState == LOW);
}

void setMotorState(bool motorOn)
{
  isMotorOn = motorOn;
  currentMotorSpeed = 0;
  analogWrite(MOTOR_CONTROL_PWM_PIN, 0);
  digitalWrite(BUITLIN_LED_PIN, motorOn ? HIGH : LOW);
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
  pinMode(ON_OFF_BUTTON_PIN, INPUT_PULLUP);

  // Start with motor off - This will reset the built in LED off indicating initialization is complete
  setMotorState(false);
}

void loop() {
  if (getOnOffPressed())
  {
    setMotorState(!isMotorOn);
  }

  if (isMotorOn)
  {
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
}