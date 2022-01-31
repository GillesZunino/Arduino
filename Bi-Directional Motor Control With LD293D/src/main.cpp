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


long minimumPwmDutyForMotorStart = 0;

bool isLeft = true;
long currentMotorSpeed = 0;


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

long getMinimumPwmDuty(float motorStartVoltage, float powerRailVoltage)
{
  // Calculate the PWM duty cycle needed to apply the motor's start voltage
  // Duty cycle = Square (Vstart / VpowerRail)
  double pwmDutyInPercentage = square(motorStartVoltage / powerRailVoltage);

  // analogWrite() encodes PWM between 0 and 255 - Map the PWN duty to this interval
  return ceil(255.0 * pwmDutyInPercentage);
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

  // Theorically: Power rail is 5V and DC motor starting voltage is 2V on the datasheet -> PWM duty for start = 16% = square(2 / 5).
  // We measured our power supply at about 4.9V and the DC motor starting voltage at about 2.2V -> We observe a start at about PWM duty = 31%, implying a starting voltage of about 2.8V with a 4.9V power rail
  // The circuit had a 0.1uF capacitor between both leads to the DC motor and one 0.1uF capacitor from each lead to ground.
  minimumPwmDutyForMotorStart = getMinimumPwmDuty(2.0f, 5.0f);

}

void loop() {
  bool hasChanged = getDirection();
  if (hasChanged)
  {
    digitalWrite(BUITLIN_LED_PIN, isLeft ? HIGH : LOW);
    setDirection(isLeft);
  }

  // Read speed from potentiometer - It will be betwen 0 and 1023 since the Analog to Digital conversion is 10 bits
  // Consider a reading or 0 on the potentiometer as "stop" and anything above 0 as the minimum PWN
  int speedPotentiometerValue = analogRead(SPEED_ANALOG_PIN);
  long requestedSpeed = map(speedPotentiometerValue, 0, 1023, speedPotentiometerValue == 0 ? 0 : minimumPwmDutyForMotorStart, 255);

  if (currentMotorSpeed != requestedSpeed)
  {
    currentMotorSpeed = requestedSpeed;
    analogWrite(MOTOR_CONTROL_PWM_PIN, requestedSpeed);
  }
}