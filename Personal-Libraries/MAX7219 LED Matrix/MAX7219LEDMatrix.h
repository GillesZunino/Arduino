#include <Arduino.h>


class MAX7219LEDMatrix {
    public:
        MAX7219LEDMatrix(int din, int cs, int clk);

        MAX7219LEDMatrix(const MAX7219LEDMatrix &) = delete;
        MAX7219LEDMatrix(MAX7219LEDMatrix &&) = delete;

        MAX7219LEDMatrix & operator=(MAX7219LEDMatrix &&) = delete;
        MAX7219LEDMatrix & operator=(const MAX7219LEDMatrix &) = delete;

    public:
        void initialize();
        void shutdown(bool on);
        void displayTest(bool on);

        // Minimum 0x00 - Max 0x0F - All values in between available
        void setBrightness(uint8_t level);

        void setRow(int row, uint8_t value);
        void setColumn(int column, uint8_t value);
        void set(int row, int column, bool on);
        void clearDisplay();

    private:
        void configure();
        void spiSendCommand(uint8_t command, uint8_t data);

    private:
        int dinPin;
        int csPin;
        int clkPin;

        uint8_t currentRows[8] = { 0x00 };

    private:
        static const uint8_t Command_TestMode = 0x0F;
        static const uint8_t Command_Shutdown = 0x0C;
        static const uint8_t Command_ScanLimit = 0x0B;
        static const uint8_t Command_Intensity = 0x0A;
        static const uint8_t Command_DecodeMode = 0x09;
};