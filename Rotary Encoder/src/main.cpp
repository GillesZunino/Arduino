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



#ifdef ENCODER_POLLING

bool detectRotaryEncoderChanges(int& position, TurnDirection& direction) {
  static int currentEncoderPosition = 0;
  static TurnDirection currentEncoderDirection = TurnDirection::Unknown;

  static int lastCLKValue = HIGH;
  static int lastSwitchValue = HIGH;

  bool hasChanged = false;

  // Handle the switch to reset all values
  int SwitchValue = digitalRead(PIN_ENCODER_SWITCH);
  if (SwitchValue != lastSwitchValue) {
    if (SwitchValue == LOW) {
      hasChanged = true;
      currentEncoderPosition = 0;
      currentEncoderDirection = TurnDirection::Unknown;
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
        currentEncoderPosition--;
        currentEncoderDirection = TurnDirection::CounterClockwise;
      } else {
        currentEncoderPosition++;
        currentEncoderDirection = TurnDirection::Clockwise;
      }
    }

    lastCLKValue = CLKValue;
  }


  position = currentEncoderPosition;
  direction = currentEncoderDirection;

  return hasChanged;
}

#endif


#ifdef ENCODER_INTERRUPT

volatile int privateRotaryEncoderPosition;
volatile TurnDirection privateRotaryEncoderDirection;
volatile byte privateRotaryEncoderHasChanged;


bool detectRotaryEncoderChanges(int& position, TurnDirection& direction) {
  byte encoderHasChanged;
  int currentEncoderValue;
  TurnDirection currentEncoderDirection;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    encoderHasChanged = privateRotaryEncoderHasChanged;
    currentEncoderValue = privateRotaryEncoderPosition;
    currentEncoderDirection = privateRotaryEncoderDirection;

    privateRotaryEncoderHasChanged = false;
  }

  position = currentEncoderValue;
  direction = currentEncoderDirection;

  return encoderHasChanged;
}


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
      privateRotaryEncoderHasChanged = true;

      if (DTValue != CLKValue) {
        privateRotaryEncoderPosition--;
        privateRotaryEncoderDirection = TurnDirection::CounterClockwise;
      } else {
        privateRotaryEncoderPosition++;
        privateRotaryEncoderDirection = TurnDirection::Clockwise;
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
      privateRotaryEncoderDirection = TurnDirection::Unknown;
      privateRotaryEncoderPosition = 0;
      privateRotaryEncoderHasChanged = true;
    }

    lastSwitchValue = SwitchValue;
  }
}

#endif

void setupRotaryEncoder() {
  pinMode(PIN_ENCODER_CLK, INPUT);
  pinMode(PIN_ENCODER_DT, INPUT);
  pinMode(PIN_ENCODER_SWITCH, INPUT_PULLUP);

#ifdef ENCODER_INTERRUPT
  // Initialize private globals
  privateRotaryEncoderPosition = 0;
  privateRotaryEncoderDirection = TurnDirection::Unknown;
  privateRotaryEncoderHasChanged = 0;

  // It appears digitalPinToInterrupt() seems to only be able to convert pins 1, 2, 3
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_CLK), EncoderCLKChangedHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_SWITCH), EncoderSwitchChangedHandler, CHANGE);
#endif
}


const char UnknownDirectionString[] ="????";
const char ClockwiseDirectionString[] ="  CW";
const char CounterClockwiseDirectionString[] = "C CW";

const char* getDirectionString(TurnDirection direction) {
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

void displayRotaryEncoderState(int encoderValue, TurnDirection encoderDirection) {
  Serial.print("Position: ");
  Serial.print(encoderValue);
  Serial.print(" ");
  Serial.print(getDirectionString(encoderDirection));
  Serial.println();

  switch (encoderDirection) {
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

  pinMode(RIGHT_LED, OUTPUT);
  pinMode(LEFT_LED, OUTPUT);

  setupRotaryEncoder();
}

void loop() {

#ifdef ENCODER_POLLING
  int currentEncoderValue;
  TurnDirection currentEncoderDirection;

  if (detectRotaryEncoderChanges(currentEncoderValue, currentEncoderDirection)) {
    displayRotaryEncoderState(currentEncoderValue, currentEncoderDirection);
  }
#endif

#ifdef ENCODER_INTERRUPT
  int currentEncoderValue;
  TurnDirection currentEncoderDirection;

  if (detectRotaryEncoderChanges(currentEncoderValue, currentEncoderDirection)) {
    displayRotaryEncoderState(currentEncoderValue, currentEncoderDirection);
  }
#endif

}