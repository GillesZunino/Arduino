#include <util/atomic.h>
#include <Arduino.h>


#define PIN_ENCODER_SWITCH 2
#define PIN_ENCODER_CLK 3
#define PIN_ENCODER_DT 4


#define RIGHT_LED 8
#define LEFT_LED 9


typedef enum {
  Unknown = 0,
  Clockwise = 1,
  CounterClockwise = 2 
} TurnDirection;


//
// Choose the mode of operation by #define only one of the following:
// * ENCODER_POLLING: Polls CLK pin and SW pin
// * ENCODER_INTERRUPT: Attach 'CHANGE' interupts to CLK pin and SW pin
//
//#define ENCODER_POLLING
#define ENCODER_INTERRUPT


volatile TurnDirection direction = TurnDirection::Unknown;
volatile int encoderPosition = 0;


#ifdef ENCODER_INTERRUPT
volatile byte encoderValueHasChanged;
#endif



#ifdef ENCODER_POLLING

bool detectEncoderChanged() {
  static int lastCLKValue = HIGH;
  static int lastSwitchValue = HIGH;

  bool hasChanged = false;

  // Handle the switch to reset all values
  int SwitchValue = digitalRead(PIN_ENCODER_SWITCH);
  if (SwitchValue != lastSwitchValue) {
    if (SwitchValue == LOW) {
      hasChanged = true;
      encoderPosition = 0;
      direction = TurnDirection::Unknown;
    }

    lastSwitchValue = SwitchValue;
  }

  //
  // For evey single encoder 'click', CLK will first go HIGH to LOW and then from LOW to HIGH
  // We need to react to one transition only or we will double count 'clicks'
  // We choose to the HIGH to LOW transition since CLK is HIGH when the encoder is idle and switches to LOW when the encoder is manipulated
  //
  int CLKValue = digitalRead(PIN_ENCODER_CLK);
  int DTValue = digitalRead(PIN_ENCODER_DT);

  if (CLKValue != lastCLKValue) {
    if (CLKValue == LOW) {
      hasChanged = true;
    
      if (DTValue != CLKValue) {
        encoderPosition--;
        direction = TurnDirection::CounterClockwise;
      } else {
        encoderPosition++;
        direction = TurnDirection::Clockwise;
      }
    }

    lastCLKValue = CLKValue;
  }

  return hasChanged;
}

#endif


#ifdef ENCODER_INTERRUPT

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
      encoderValueHasChanged = true;

      if (DTValue != CLKValue) {
        encoderPosition--;
        direction = TurnDirection::CounterClockwise;
      } else {
        encoderPosition++;
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
      encoderPosition = 0;
      encoderValueHasChanged = true;
    }

    lastSwitchValue = SwitchValue;
  }
}

#endif


const char UnknownDirectionString[] ="????";
const char ClockwiseDirectionString[] ="  CW";
const char CounterClockwiseDirectionString[] = "C CW";

const char* getDirectionString() {
    switch (direction) {
    case TurnDirection::Clockwise:
      return ClockwiseDirectionString;

    case TurnDirection::CounterClockwise:
      return CounterClockwiseDirectionString;

    case TurnDirection::Unknown:
    default:
      return UnknownDirectionString;
  }
}

void displayState(int currentEncoderValue, TurnDirection currentEncoderDirection) {
  Serial.print("Position: ");
  Serial.print(currentEncoderValue);
  Serial.print(" ");
  Serial.print(getDirectionString());
  Serial.println();

  switch (currentEncoderDirection) {
    case TurnDirection::Clockwise:
      digitalWrite(RIGHT_LED, HIGH);
      digitalWrite(LEFT_LED, LOW);
    break;

    case TurnDirection::CounterClockwise:
      digitalWrite(RIGHT_LED, LOW);
      digitalWrite(LEFT_LED, HIGH);
    break;

    case TurnDirection::Unknown:
    default:
      digitalWrite(RIGHT_LED, LOW);
      digitalWrite(LEFT_LED, LOW);
    break;
  }
}


void setup() {
  Serial.begin (9600);

  pinMode(PIN_ENCODER_CLK, INPUT);
  pinMode(PIN_ENCODER_DT, INPUT);
  pinMode(PIN_ENCODER_SWITCH, INPUT_PULLUP);

  pinMode(RIGHT_LED, OUTPUT);
  pinMode(LEFT_LED, OUTPUT);

#ifdef ENCODER_INTERRUPT
  encoderValueHasChanged = false;

  // It appears digitalPinToInterrupt() seems to only be able to convert pins 1, 2, 3
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_CLK), EncoderCLKChangedHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_SWITCH), EncoderSwitchChangedHandler, CHANGE);
#endif

  displayState(encoderPosition, direction);
}

void loop() {

#ifdef ENCODER_POLLING
  if (detectEncoderChanged()) {
    displayState(encoderPosition, direction);
  }
#endif

#ifdef ENCODER_INTERRUPT
  byte encoderChanged = false;
  int currentEncoderValue = 0;
  TurnDirection currentEncoderDirection;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    encoderChanged = encoderValueHasChanged;
    currentEncoderValue = encoderPosition;
    currentEncoderDirection = direction;
    encoderValueHasChanged = false;
  }

  if (encoderChanged) {
    displayState(currentEncoderValue, currentEncoderDirection);
  }

  delay(1);
#endif

}