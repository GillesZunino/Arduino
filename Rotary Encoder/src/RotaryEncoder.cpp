#include <util/atomic.h>
#include "RotaryEncoder.h"


#ifdef ENCODER_INTERRUPTS
byte RotaryEncoder::nextEncoderIndex = 0;
RotaryEncoder::EncoderState RotaryEncoder::rotaryEncoders[ENCODERS_COUNT] = {};
#endif


RotaryEncoder::RotaryEncoder(uint8_t clkPin, uint8_t dtPin, uint8_t swPin)
{
#ifdef ENCODER_POLLING
    this->clkPin = clkPin;
    this->dtPin = dtPin;
    this->swPin = swPin;

    lastCLKValue = HIGH;
    lastSwitchValue = HIGH;

    encoderPosition = 0;
    encoderDirection = TurnDirection::Unknown;
    encoderHasChanged = 0;
#endif

#ifdef ENCODER_INTERRUPTS
    currentEncoderIndex = RotaryEncoder::nextEncoderIndex++;
    RotaryEncoder::rotaryEncoders[currentEncoderIndex].clkPin = clkPin;
    RotaryEncoder::rotaryEncoders[currentEncoderIndex].dtPin = dtPin;
    RotaryEncoder::rotaryEncoders[currentEncoderIndex].swPin = swPin;

    RotaryEncoder::rotaryEncoders[currentEncoderIndex].lastCLKValue = HIGH;
    RotaryEncoder::rotaryEncoders[currentEncoderIndex].lastSwitchValue = HIGH;

    RotaryEncoder::rotaryEncoders[currentEncoderIndex].encoderPosition = 0;
    RotaryEncoder::rotaryEncoders[currentEncoderIndex].encoderDirection = RotaryEncoder::TurnDirection::Unknown;
    RotaryEncoder::rotaryEncoders[currentEncoderIndex].encoderHasChanged = 0;
#endif
}

void RotaryEncoder::initialize()
{
#ifdef ENCODER_POLLING
    pinMode(clkPin, INPUT);
    pinMode(dtPin, INPUT);
    pinMode(swPin, INPUT_PULLUP);
#endif

#ifdef ENCODER_INTERRUPTS
    pinMode(RotaryEncoder::rotaryEncoders[currentEncoderIndex].clkPin, INPUT);
    pinMode(RotaryEncoder::rotaryEncoders[currentEncoderIndex].dtPin, INPUT);
    pinMode(RotaryEncoder::rotaryEncoders[currentEncoderIndex].swPin, INPUT_PULLUP);

    // It appears digitalPinToInterrupt() seems to only be able to convert pins 1, 2, 3
    // TODO: Support multiple encoders - We might need to attach the interrupt only once and detect in the ISR
    attachInterrupt(digitalPinToInterrupt(RotaryEncoder::rotaryEncoders[currentEncoderIndex].clkPin), RotaryEncoder::EncoderCLKChangedISRHandler, CHANGE);
    attachInterrupt(digitalPinToInterrupt(RotaryEncoder::rotaryEncoders[currentEncoderIndex].swPin), RotaryEncoder::EncoderSwitchChangedISRHandler, CHANGE);
#endif
}

#ifdef ENCODER_INTERRUPTS

bool RotaryEncoder::DetectRotaryEncoderChanges(int &position, TurnDirection &direction)
{
    byte encoderHasChanged;
    int currentEncoderValue;
    TurnDirection currentEncoderDirection;

    EncoderState *currentEncoderState = &rotaryEncoders[currentEncoderIndex];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        encoderHasChanged = currentEncoderState->encoderHasChanged;
        currentEncoderValue = currentEncoderState->encoderPosition;
        currentEncoderDirection = currentEncoderState->encoderDirection;

        currentEncoderState->encoderHasChanged = false;
    }

    position = currentEncoderValue;
    direction = currentEncoderDirection;

    return encoderHasChanged;
}

void RotaryEncoder::EncoderCLKChangedISRHandler()
{
    // TODO: Locate the correct encoder state record if there is more than one encoder
    EncoderState* currentEncoder = &RotaryEncoder::rotaryEncoders[0];

    int CLKValue = digitalRead(currentEncoder->clkPin);
    int DTValue = digitalRead(currentEncoder->dtPin);

    //
    // For evey single encoder 'click', CLK will first go HIGH to LOW and then from LOW to HIGH
    // We need to react to one transition only or we will double count 'clicks'
    // We choose to the HIGH to LOW transition since CLK is HIGH when the encoder is idle and switches to LOW when the encoder is manipulated
    //
    if (CLKValue != currentEncoder->lastCLKValue)
    {
        if (CLKValue == LOW)
        {
            currentEncoder->encoderHasChanged = true;

            if (DTValue != CLKValue)
            {
                currentEncoder->encoderPosition--;
                currentEncoder->encoderDirection = TurnDirection::CounterClockwise;
            }
            else
            {
                currentEncoder->encoderPosition++;
                currentEncoder->encoderDirection = TurnDirection::Clockwise;
            }
        }

        currentEncoder->lastCLKValue = CLKValue;
    }
}

void RotaryEncoder::EncoderSwitchChangedISRHandler()
{
    // TODO: Locate the correct encoder state record if there is more than one encoder
    EncoderState* currentEncoder = &RotaryEncoder::rotaryEncoders[0];

    int SwitchValue = digitalRead(currentEncoder->swPin);
    if (SwitchValue != currentEncoder->lastSwitchValue)
    {
        // Switch is active LOW
        if (SwitchValue == LOW)
        {
            currentEncoder->encoderDirection = TurnDirection::Unknown;
            currentEncoder->encoderPosition = 0;

            currentEncoder->encoderHasChanged = true;
        }

        currentEncoder->lastSwitchValue = SwitchValue;
    }
}

#endif

#ifdef ENCODER_POLLING

bool RotaryEncoder::DetectRotaryEncoderChanges(int &position, TurnDirection &direction)
{
    bool hasChanged = false;

    // Handle the switch to reset all values
    int switchValue = digitalRead(swPin);
    if (switchValue != lastSwitchValue)
    {
        if (switchValue == LOW)
        {
            hasChanged = true;
            encoderPosition = 0;
            encoderDirection = TurnDirection::Unknown;
        }

        lastSwitchValue = switchValue;
    }

        //
    // For evey single encoder 'click', CLK will first go HIGH to LOW and then from LOW to HIGH
    // We need to react to one transition only or we will double count 'clicks'
    // We choose to the HIGH to LOW transition since CLK is HIGH when the encoder is idle and switches to LOW when the encoder is manipulated
    //
    int CLKValue = digitalRead(clkPin);
    int DTValue = digitalRead(dtPin);

    if (CLKValue != lastCLKValue)
    {
        if (CLKValue == LOW)
        {
            hasChanged = true;

            if (DTValue != CLKValue)
            {
                encoderPosition--;
                encoderDirection = TurnDirection::CounterClockwise;
            }
            else
            {
                encoderPosition++;
                encoderDirection = TurnDirection::Clockwise;
            }
        }

        lastCLKValue = CLKValue;
    }

    position = encoderPosition;
    direction = encoderDirection;

    return hasChanged;
}

#endif