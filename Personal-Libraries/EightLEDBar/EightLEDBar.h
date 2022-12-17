#include <Arduino.h>

class EightLEDBar {
    public:
        EightLEDBar(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);

        void displayPercent(uint8_t percent);
        void displayRaw(uint8_t rawValue);
        
    public:
        static const uint8_t LEDCount = 8;

    public:
        // No LED on
        static const uint8_t NONE = 0;

        // Individual LED addresses
        static const uint8_t BLUE_1 = 0x01;
        static const uint8_t BLUE_2 = 0x02;
        static const uint8_t GREEN_1 = 0x04;
        static const uint8_t GREEN_2 = 0x08;
        static const uint8_t YELLOW_1 = 0x10;
        static const uint8_t YELLOW_2 = 0x20;
        static const uint8_t RED_1 = 0x40;
        static const uint8_t RED_2 = 0x80;

    private:
        uint8_t dataPin;
        uint8_t clockPin;
        uint8_t latchPin;
};