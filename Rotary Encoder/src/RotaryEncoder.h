#include <Arduino.h>

//
// Choose the mode of operation by #define only one of the following:
// * ENCODER_POLLING: Polls CLK pin and SW pin - Can have as many rotary encoder conneted as memory / GPIO ports allow
// * ENCODER_INTERRUPTS: Attach 'CHANGE' interupts to CLK pin and SW pin
//                       Define ENCODERS_COUNT to the number of encoders connected
//
#define ENCODER_INTERRUPTS
#define ENCODERS_COUNT 1



#if defined(ENCODER_INTERRUPTS) && !defined(ENCODERS_COUNT)
#error "When using ENCODER_INTERRUPTS, ENCODERS_COUNT must de defined to the number of encoders in your system"
#endif

// TODO: Support more than one encoder
#if defined(ENCODERS_COUNT) && defined(ENCODERS_COUNT) && (ENCODERS_COUNT != 1)
#error "Only one encoder is currently supported - Use #define ENCODERS_COUNT 1"
#endif

class RotaryEncoder
{
public:
    typedef enum
    {
        Unknown = 0,
        Clockwise = 1,
        CounterClockwise = 2
    } TurnDirection;

public:
    RotaryEncoder(uint8_t clkPin, uint8_t dtPin, uint8_t swPin);

    RotaryEncoder(const RotaryEncoder &) = delete;
    RotaryEncoder(RotaryEncoder &&) = delete;

    RotaryEncoder & operator=(RotaryEncoder &&) = delete;
    RotaryEncoder & operator=(const RotaryEncoder &) = delete;

public:
    void initialize();
    bool DetectRotaryEncoderChanges(int &position, TurnDirection &direction);

private:
#ifdef ENCODER_INTERRUPTS
    static void EncoderCLKChangedISRHandler();
    static void EncoderSwitchChangedISRHandler();
#endif

private:
#ifdef ENCODER_POLLING
    void EncoderCLKChangedISRHandler();
    void EncoderSwitchChangedISRHandler();
#endif

private:
#ifdef ENCODER_INTERRUPTS
    struct EncoderState
    {
        int clkPin;
        int dtPin;
        int swPin;

        int lastCLKValue;
        int lastSwitchValue;

        volatile int encoderPosition;
        volatile RotaryEncoder::TurnDirection encoderDirection;
        volatile byte encoderHasChanged;
    };

    int currentEncoderIndex;

    static byte nextEncoderIndex;
    static EncoderState rotaryEncoders[ENCODERS_COUNT];
#endif

private:
#ifdef ENCODER_POLLING
    int clkPin;
    int dtPin;
    int swPin;

    int lastCLKValue;
    int lastSwitchValue;

    int encoderPosition;
    TurnDirection encoderDirection;
    byte encoderHasChanged;
#endif
};