#include <Arduino.h>
#include <Servo.h>

#include <RotaryEncoder.h>


#if DEBUG
#include "avr8-stub.h"
#include "app_api.h"
#endif


#define PIN_ENCODER_SWITCH 2
#define PIN_ENCODER_CLK 3
#define PIN_ENCODER_DT 4

RotaryEncoder rotaryEncoder(PIN_ENCODER_CLK, PIN_ENCODER_DT, PIN_ENCODER_SWITCH);


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
  rotaryEncoder.initialize();

  // Initialize the servo library and move the servo to center position (90°)
  servo.attach(SERVO_PWM_PIN);
  servo.write(SERVO_INITIAL_ANGLE);
}

void loop() {
  int currentEncoderValue;
  RotaryEncoder::TurnDirection currentEncoderDirection;

  if (rotaryEncoder.DetectRotaryEncoderChanges(currentEncoderValue, currentEncoderDirection)) {
    //
    // The servo we use can go from 0° to 180° - We choose 90° to be the 'middle'
    // * 0°  : Index is located on the right of the servo at 90° (servo wire oriented up)
    // * 90° : Index is located straight up (servo wire oriented up)
    // * 180 : Index is located on the left of the servo at 90° (servo wire oriented up)
    //
    bool clockWise = currentEncoderValue > 0;
    int clampedRotaryEncoderAngle = abs(currentEncoderValue) <= SERVO_INITIAL_ANGLE ? abs(currentEncoderValue) : SERVO_INITIAL_ANGLE;
    int servoAngle = clockWise ? SERVO_INITIAL_ANGLE - clampedRotaryEncoderAngle : SERVO_INITIAL_ANGLE + clampedRotaryEncoderAngle;
    servo.write(servoAngle);
  }
}