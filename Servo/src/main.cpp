#include <util/atomic.h>
#include <Arduino.h>
#include <Servo.h>



#if DEBUG
#include "avr8-stub.h"
#include "app_api.h"
#endif


// Rotary Encoder --------------------------------------------------------------------------------------------------------------------------------------

#define PIN_ENCODER_SWITCH 2
#define PIN_ENCODER_CLK 3
#define PIN_ENCODER_DT 4


typedef enum {
  Unknown = 0,
  Clockwise = 1,
  CounterClockwise = 2 
} TurnDirection;


volatile TurnDirection direction = TurnDirection::Unknown;
volatile int rotaryEncoderPosition = 0;

volatile byte rotaryEncoderValueHasChanged;

void EncoderCLKChangedHandler()
{
  static int lastCLKValue = HIGH;

  int CLKValue = digitalRead(PIN_ENCODER_CLK);
  int DTValue = digitalRead(PIN_ENCODER_DT);

  //
  // For evey single encoder 'click', CLK will first go HIGH to LOW and then from LOW to HIGH
  // We need to react to one transition only or we will double count 'clicks'
  // We choose to the HIGH to LOW transition since CLK is HIGH when the encoder is idle and switches to LOW when the encoder is manipulated
  //
  if (CLKValue != lastCLKValue) {
    if (CLKValue == LOW) {
      rotaryEncoderValueHasChanged = true;

      if (DTValue != CLKValue) {
        rotaryEncoderPosition--;
        direction = TurnDirection::CounterClockwise;
      } else {
        rotaryEncoderPosition++;
        direction = TurnDirection::Clockwise;
      }
    }

    lastCLKValue = CLKValue;
  }
}

void EncoderSwitchChangedHandler() {
  static int lastSwitchValue = HIGH;

  int SwitchValue = digitalRead(PIN_ENCODER_SWITCH);
  if (SwitchValue != lastSwitchValue) {
    // Switch is active LOW
    if (SwitchValue == LOW) {
      direction = TurnDirection::Unknown;
      rotaryEncoderPosition = 0;
      rotaryEncoderValueHasChanged = true;
    }

    lastSwitchValue = SwitchValue;
  }
}

void setupRotaryEncoder() {
  pinMode(PIN_ENCODER_CLK, INPUT);
  pinMode(PIN_ENCODER_DT, INPUT);
  pinMode(PIN_ENCODER_SWITCH, INPUT_PULLUP);

  rotaryEncoderValueHasChanged = false;

  // It appears digitalPinToInterrupt() seems to only be able to convert pins 1, 2, 3
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_CLK), EncoderCLKChangedHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_SWITCH), EncoderSwitchChangedHandler, CHANGE);
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------


//
// The servo we use has 3 wires:
//  * Orange: PWM
//  * Red:    Vcc 5V
//  * Black:  GND
//
#define SERVO_PWM_PIN 9

#define SERVO_INITIAL_ANGLE 90
#define SERVO_MAX_ANGLE 180


Servo servo;

void setup() {

#if DEBUG
  // Initialize GDB stub - The debugger uses INT0 [pin 2 (Uno) or pin 21 (Mega)] so DO NOT USE these pins !
  debug_init();
#endif

  // Initialize rotary encoder
  setupRotaryEncoder();

  // Initialize the servo library and move the servo to center position (90°)
  servo.attach(SERVO_PWM_PIN);
  servo.write(SERVO_INITIAL_ANGLE);
}

void loop() {
  int currentRotaryEncoderValue;
  byte hasNewRotaryEncoderValue = false;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    hasNewRotaryEncoderValue = rotaryEncoderValueHasChanged;
    currentRotaryEncoderValue = rotaryEncoderPosition;
    rotaryEncoderValueHasChanged = false;
  }

  if (hasNewRotaryEncoderValue) {
    //
    // The servo we use can go from 0° to 180° - We choose 90° to be the 'middle'
    // * 0°  : Index is located on the right of the servo at 90° (servo wire oriented up)
    // * 90° : Index is located straight up (servo wire oriented up)
    // * 180 : Index is located on the left of the servo at 90° (servo wire oriented up)
    //
    bool clockWise = currentRotaryEncoderValue > 0;
    int clampedRotaryEncoderAngle = abs(currentRotaryEncoderValue) <= SERVO_INITIAL_ANGLE ? abs(currentRotaryEncoderValue) : SERVO_INITIAL_ANGLE;
    int servoAngle = clockWise ? SERVO_INITIAL_ANGLE - clampedRotaryEncoderAngle : SERVO_INITIAL_ANGLE + clampedRotaryEncoderAngle;
    servo.write(servoAngle);
  }
}