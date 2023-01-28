#include <Arduino.h>


class EightSevenSegmentDisplays {
    public:
        EightSevenSegmentDisplays(int din, int cs, int clk);

        EightSevenSegmentDisplays(const EightSevenSegmentDisplays &) = delete;
        EightSevenSegmentDisplays(EightSevenSegmentDisplays &&) = delete;

        EightSevenSegmentDisplays & operator=(EightSevenSegmentDisplays &&) = delete;
        EightSevenSegmentDisplays & operator=(const EightSevenSegmentDisplays &) = delete;

    public:
        void initialize();
        void shutdown(bool on);

        void displayTest(bool on);

        // Minimum 0x00 - Max 0x0F - All values in between available
        void setBrightness(uint8_t level);

        void clearDisplay();

        // Digit from 0 (leftmost) to 7 (rightmost)
        void set(uint8_t digit, uint8_t value);
        void setCharacter(uint8_t digit, char character, bool decimalPoint);
        void setDecimalPoint(uint8_t digit, bool on);

    private:
        int mapCharacterToBinaryPattern(char character, bool decimalPoint);

    private:
        void configure();
        void spiSendCommand(uint8_t command, uint8_t data);

    private:
        int dinPin;
        int csPin;
        int clkPin;

        uint8_t currentDigits[8] = { 0x00 };

    private:
        static const uint8_t Command_TestMode = 0x0F;
        static const uint8_t Command_Shutdown = 0x0C;
        static const uint8_t Command_ScanLimit = 0x0B;
        static const uint8_t Command_Intensity = 0x0A;
        static const uint8_t Command_DecodeMode = 0x09;
};