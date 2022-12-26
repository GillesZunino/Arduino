#include <util/atomic.h>
#include <Arduino.h>

#include "RotaryEncoder.h"


#define PIN_ENCODER_SWITCH 2
#define PIN_ENCODER_CLK 3
#define PIN_ENCODER_DT 4


#define RIGHT_LED 8
#define LEFT_LED 9


RotaryEncoder rotaryEncoder(PIN_ENCODER_CLK, PIN_ENCODER_DT, PIN_ENCODER_SWITCH);


const char UnknownDirectionString[] ="????";
const char ClockwiseDirectionString[] ="  CW";
const char CounterClockwiseDirectionString[] = "C CW";

const char* getDirectionString(RotaryEncoder::TurnDirection direction) {
    switch (direction) {
    case RotaryEncoder::TurnDirection::Clockwise:
      return ClockwiseDirectionString;

    case RotaryEncoder::TurnDirection::CounterClockwise:
      return CounterClockwiseDirectionString;

    case RotaryEncoder::TurnDirection::Unknown:
    default:
      return UnknownDirectionString;
  }
}

void displayRotaryEncoderState(int encoderValue, RotaryEncoder::TurnDirection encoderDirection) {
  Serial.print("Position: ");
  Serial.print(encoderValue);
  Serial.print(" ");
  Serial.print(getDirectionString(encoderDirection));
  Serial.println();

  switch (encoderDirection) {
    case RotaryEncoder::TurnDirection::Clockwise:
      digitalWrite(RIGHT_LED, HIGH);
      digitalWrite(LEFT_LED, LOW);
    break;

    case RotaryEncoder::TurnDirection::CounterClockwise:
      digitalWrite(RIGHT_LED, LOW);
      digitalWrite(LEFT_LED, HIGH);
    break;

    case RotaryEncoder::TurnDirection::Unknown:
    default:
      digitalWrite(RIGHT_LED, LOW);
      digitalWrite(LEFT_LED, LOW);
    break;
  }
}


void setup() {
  Serial.begin (115200);

  pinMode(RIGHT_LED, OUTPUT);
  pinMode(LEFT_LED, OUTPUT);

  rotaryEncoder.initialize();
}

void loop() {
  int currentEncoderValue;
  RotaryEncoder::TurnDirection currentEncoderDirection;

  if (rotaryEncoder.DetectRotaryEncoderChanges(currentEncoderValue, currentEncoderDirection)) {
    displayRotaryEncoderState(currentEncoderValue, currentEncoderDirection);
  }
}