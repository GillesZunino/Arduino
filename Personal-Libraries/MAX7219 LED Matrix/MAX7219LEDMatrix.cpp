#include <MAX7219LEDMatrix.h>


MAX7219LEDMatrix::MAX7219LEDMatrix(int din, int cs, int clk) {
    dinPin = din;
    csPin = cs;
    clkPin = clk;
}

void MAX7219LEDMatrix::initialize() {
    pinMode(dinPin, OUTPUT);
    pinMode(csPin, OUTPUT);
    pinMode(clkPin, OUTPUT);

    // Configure the MAX7219 chip
    configure();
}

void MAX7219LEDMatrix::shutdown(bool on) {
    if (on) {
        // Mode: SHUTDOWN
        spiSendCommand(Command_Shutdown, 0x00);
    } else {
        // Mode: NORMAL
        spiSendCommand(Command_Shutdown, 0x01);
    }
}

void MAX7219LEDMatrix::displayTest(bool on) {
    if (on) {
        spiSendCommand(Command_TestMode, 0x01);
    } else {
        spiSendCommand(Command_TestMode, 0x00);
    }
}

void MAX7219LEDMatrix::setBrightness(uint8_t level) {
    if ((level >= 0x00) && (level <= 0x0F)) {
        spiSendCommand(Command_Intensity, level);
    }
}

void MAX7219LEDMatrix::setRow(int row, uint8_t value) {
    if ((row >= 0) && (row <= 7)) {
        currentRows[row] = value;
        spiSendCommand(row + 1, value);
    }
}

void MAX7219LEDMatrix::setColumn(int column, uint8_t value) {
    if ((column >= 0) && (column <= 7)) {
        for (int row = 0; row < 8; row++) {
            if (value & 0x80) {
                currentRows[row] |= 1 << (7 - column);
            } else {
                currentRows[row] &= ~(1 << (7 - column));
            }
            value <<= 1;
        }

        for (int row = 0; row < sizeof(currentRows) / sizeof(currentRows[0]; row++) {
            spiSendCommand(row + 1, currentRows[row]);
        }
    }
}

void MAX7219LEDMatrix::set(int row, int column, bool on) {
    if ((row >= 0) && (row <= 7)) {
        if ((column >= 0) && (column <= 7)) {
            if (on) {
                currentRows[row] |= 1 << (7 - column);
            } else {
                currentRows[row] &= ~(1 << (7 - column));
            }

            for (int row = 0; row < sizeof(currentRows) / sizeof(currentRows[0]; row++) {
                spiSendCommand(row + 1, currentRows[row]);
            }
        }
    }
}

void MAX7219LEDMatrix::clearDisplay() {
    for (int row = 0; row < sizeof(currentRows) / sizeof(currentRows[0]); row++) {
        currentRows[row] = 0x00;
        spiSendCommand(row + 1, 0x00);
    }
}

void MAX7219LEDMatrix::configure() {
    // DISPLAY TEST MODE: 0 (OFF)
    spiSendCommand(Command_TestMode, 0x00);

    // SCAN LIMIT: 7
    spiSendCommand(Command_ScanLimit, 0x07);

    // DECODE: No Decode
    spiSendCommand(Command_DecodeMode, 0x00);

    // Mode: SHUTDOWN
    spiSendCommand(Command_Shutdown, 0x00);
}

void MAX7219LEDMatrix::spiSendCommand(uint8_t command, uint8_t data) {
    digitalWrite(csPin, LOW);

    shiftOut(dinPin, clkPin, MSBFIRST, command);
    shiftOut(dinPin, clkPin, MSBFIRST, data);
    
    digitalWrite(csPin, HIGH);
}