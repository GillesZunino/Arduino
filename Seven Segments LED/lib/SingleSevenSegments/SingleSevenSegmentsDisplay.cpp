
#include "SingleSevenSegmentsDisplay.h"


const uint8_t SingleSevenSegmentsDisplay::digitToSevenSegmentsMap[] = {
    B11111100,  // = 0
    B01100000,  // = 1
    B11011010,  // = 2
    B11110010,  // = 3
    B01100110,  // = 4
    B10110110,  // = 5
    B10111110,  // = 6
    B11100000,  // = 7
    B11111110,  // = 8
    B11100110   // = 9
};



SingleSevenSegmentsDisplay::SingleSevenSegmentsDisplay(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin)
{
    this->dataPin = dataPin;
    this->clockPin = clockPin;
    this->latchPin = latchPin;

    pinMode(this->dataPin, OUTPUT);
    pinMode(this->clockPin, OUTPUT);
    pinMode(this->latchPin, OUTPUT);

    rawSend(0);
}

void SingleSevenSegmentsDisplay::displayDigit(uint8_t digit, bool decimalPoint)
{
    if ((digit >= 0) && (digit < sizeof(digitToSevenSegmentsMap) / sizeof(digitToSevenSegmentsMap[0])))
    {
        uint8_t toSend = digitToSevenSegmentsMap[digit] | (decimalPoint ? 1 : 0);
        rawSend(toSend);
    }
}


void SingleSevenSegmentsDisplay::displayLetter(uint8_t letter, bool decimalPoint)
{
    uint8_t decodedLetter = 0;
    switch (letter)
    {
        case 'A':
        decodedLetter = B11101110;
        break;
        case 'C':
        decodedLetter = B10011100;
        break;
        case 'E':
        decodedLetter = B10011110;
        break;
        case 'F':
        decodedLetter = B10001110;
        break;
        case 'H':
        decodedLetter = B01101110;
        break;
        case 'J':
        decodedLetter = B01111000;
        break;
        case 'L':
        decodedLetter = B00011100;
        break;
        case 'P':
        decodedLetter = B11001110;
        break;
        case 'U':
        decodedLetter = B01111100;
        break;
    }

    if (decodedLetter != 0)
    {
        uint8_t toSend = decodedLetter | (decimalPoint ? 1 : 0);
        rawSend(toSend);
    }
}

void SingleSevenSegmentsDisplay::rawSend(uint8_t rawValue)
{
    // latchPin to LOW
    digitalWrite(latchPin, LOW);

    // Send the data
    shiftOut(dataPin, clockPin, LSBFIRST, rawValue);  

    // Transfer data to storage register by setting latchPin to HIGH
    digitalWrite(latchPin, HIGH);
}