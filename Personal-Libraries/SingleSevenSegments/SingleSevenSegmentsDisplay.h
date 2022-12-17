#include <Arduino.h>

class SingleSevenSegmentsDisplay {
    public:
        SingleSevenSegmentsDisplay(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);
        
        void displayDigit(uint8_t digit, bool decimalPoint);
        void displayLetter(uint8_t letter, bool decimalPoint);

    private:
        void rawSend(uint8_t rawValue);

    private:
        uint8_t dataPin;
        uint8_t clockPin;
        uint8_t latchPin;

    private:
        static const uint8_t digitToSevenSegmentsMap[];
};