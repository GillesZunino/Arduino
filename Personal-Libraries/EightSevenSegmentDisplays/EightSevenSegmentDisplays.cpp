#include <EightSevenSegmentDisplays.h>


EightSevenSegmentDisplays::EightSevenSegmentDisplays(int din, int cs, int clk) {
    dinPin = din;
    csPin = cs;
    clkPin = clk;
}

void EightSevenSegmentDisplays::initialize() {
    pinMode(dinPin, OUTPUT);
    pinMode(csPin, OUTPUT);
    pinMode(clkPin, OUTPUT);

    // Set LOAD / (CS) to high to avoid triggering the MAX7219 chip
    digitalWrite(csPin, HIGH);

    // Configure the MAX7219 chip
    configure();
}

void EightSevenSegmentDisplays::shutdown(bool on) {
    if (on) {
        // Mode: SHUTDOWN
        spiSendCommand(Command_Shutdown, 0x00);
    } else {
        // Mode: NORMAL
        spiSendCommand(Command_Shutdown, 0x01);
    }
}

void EightSevenSegmentDisplays::displayTest(bool on) {
    if (on) {
        spiSendCommand(Command_TestMode, 0x01);
    } else {
        spiSendCommand(Command_TestMode, 0x00);
    }
}

void EightSevenSegmentDisplays::setBrightness(uint8_t level) {
    if ((level >= 0x00) && (level <= 0x0F)) {
        spiSendCommand(Command_Intensity, level);
    }
}

void EightSevenSegmentDisplays::clearDisplay() {
    for (uint8_t digit = 0; digit < sizeof(currentDigits) / sizeof(currentDigits[0]); digit++) {
        currentDigits[digit] = 0x00;
        spiSendCommand(digit + 1, 0x00);
    }
}

void EightSevenSegmentDisplays::set(uint8_t digit, uint8_t value) {
    if ((digit >= 0) && (digit <= 7)) {
        currentDigits[digit] = value;
        spiSendCommand(digit + 1, currentDigits[digit]);
    }
}

void EightSevenSegmentDisplays::setCharacter(uint8_t digit, char character, bool decimalPoint) {
    if ((digit >= 0) && (digit <= 7)) {
        int pattern = mapCharacterToBinaryPattern(character, decimalPoint);
        if (pattern >= 0) {
            set(digit, static_cast<uint8_t>(pattern));
        }
    }
}

void EightSevenSegmentDisplays::setDecimalPoint(uint8_t digit,bool on) {
    if ((digit >= 0) && (digit <= 7)) {
        uint8_t pattern = currentDigits[digit];
        if (on) {
            pattern |= 0x80;
        } else {
            pattern &= 0x7F;
        }
        set(digit, static_cast<uint8_t>(pattern));
    }
}

int EightSevenSegmentDisplays::mapCharacterToBinaryPattern(char character, bool decimalPoint) {
    constexpr uint8_t DigitToSevenSegmentsMap[] = {
        B01111110,  // = 0
        B00110000,  // = 1
        B01101101,  // = 2
        B01111001,  // = 3
        B00110011,  // = 4
        B01011011,  // = 5
        B01011111,  // = 6
        B01110000,  // = 7
        B01111111,  // = 8
        B01110011   // = 9
    };

    int decodedLetter = -1;

    if ((character >= '0') && (character <= '9')) {
        decodedLetter = DigitToSevenSegmentsMap[character - '0'];
    } else {
        switch (character) {
            //
            // Segment Letters:
            //
            // - A -
            // |   |
            // F   B
            // |   |
            // - G -
            // |   |
            // E   C
            // |   |
            // - D - DP
            //
            // Segment Order: DP, A, B, C, D, E, F, G
            //
            case 'A':
            decodedLetter = B01110111;
            break;
            case 'C':
            decodedLetter = B01001110;
            break;
            case 'E':
            decodedLetter = B01001111;
            break;
            case 'F':
            decodedLetter = B01000111;
            break;
            case 'H':
            decodedLetter = B00110111;
            break;
            case 'J':
            decodedLetter = B00111100;
            break;
            case 'L':
            decodedLetter = B00001110;
            break;
            case 'P':
            decodedLetter = B01100111;
            break;
            case 'U':
            decodedLetter = B00111110;
            break;
            case '-':
            decodedLetter = B00000001;
            break;
            case ' ':
            decodedLetter = B00000000;
            break;
        }
    }

    if (decimalPoint && (decodedLetter != -1)) {
        decodedLetter |= decimalPoint ? 0x80 : 0;
    }

    return decodedLetter;
}

void EightSevenSegmentDisplays::configure() {
    // DISPLAY TEST MODE: 0 (OFF)
    spiSendCommand(Command_TestMode, 0x00);

    // SCAN LIMIT: 7
    spiSendCommand(Command_ScanLimit, 0x07);

    // DECODE: No Decode
    spiSendCommand(Command_DecodeMode, 0x00);

    // Mode: SHUTDOWN
    spiSendCommand(Command_Shutdown, 0x00);
}

void EightSevenSegmentDisplays::spiSendCommand(uint8_t command, uint8_t data) {
    digitalWrite(csPin, LOW);

    shiftOut(dinPin, clkPin, MSBFIRST, command);
    shiftOut(dinPin, clkPin, MSBFIRST, data);
    
    digitalWrite(csPin, HIGH);
}