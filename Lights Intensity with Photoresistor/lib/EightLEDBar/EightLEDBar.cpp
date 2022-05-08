#include "EightLEDBar.h"


EightLEDBar::EightLEDBar(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin) {
    this->dataPin = dataPin;
    this->clockPin = clockPin;
    this->latchPin = latchPin;

    pinMode(this->dataPin, OUTPUT);
    pinMode(this->clockPin, OUTPUT);
    pinMode(this->latchPin, OUTPUT);

    displayRaw(EightLEDBar::NONE);
}

void EightLEDBar::displayRaw(uint8_t rawValue)
{
    // latchPin to LOW
    digitalWrite(latchPin, LOW);

    // Send the data
    shiftOut(dataPin, clockPin, LSBFIRST, rawValue);  

    // Transfer data to storage register by setting latchPin to HIGH
    digitalWrite(latchPin, HIGH);
}

void EightLEDBar::displayPercent(uint8_t percent)
{
    //
    // There are 8 LEDs - One LED represents 12.5%
    // * percent = 0          -> 0 LED
    // * percent in [1..12]   -> 1 LED
    // * percent in [13..25]  -> 2 LED
    // * percent in [26..37]  -> 3 LED
    // * percent in [38..50]  -> 4 LED
    // * percent in [51..62]  -> 5 LED
    // * percent in [63..75]  -> 6 LED
    // * percent in [76..87]  -> 7 LED
    // * percent in [88..100] -> 8 LED
    // * percent >= 100       -> 8 LED
    //
    uint8_t ledMask = EightLEDBar::NONE;
    if (percent > 0)
    {
        if (percent <= 12)
        {
            ledMask = EightLEDBar::BLUE_1;
        }
        else
        {
            if ((percent > 12) && (percent <= 25))
            {
                ledMask = EightLEDBar::BLUE_1 | EightLEDBar::BLUE_2;
            }
            else
            {
                if ((percent > 25) && (percent <= 37))
                {
                    ledMask = EightLEDBar::BLUE_1 | EightLEDBar::BLUE_2 | EightLEDBar::GREEN_1;
                }
                else
                {
                    if ((percent >= 38) && (percent <= 50))
                    {
                        ledMask = EightLEDBar::BLUE_1 | EightLEDBar::BLUE_2 | EightLEDBar::GREEN_1 | EightLEDBar::GREEN_2;
                    }
                    else
                    {
                        if ((percent > 50) && (percent <= 62))
                        {
                            ledMask = EightLEDBar::BLUE_1 | EightLEDBar::BLUE_2 | EightLEDBar::GREEN_1 | EightLEDBar::GREEN_2 | EightLEDBar::YELLOW_1;
                        }
                        else
                        {
                            if ((percent > 62) && (percent <= 75))
                            {
                                ledMask = EightLEDBar::BLUE_1 | EightLEDBar::BLUE_2 | EightLEDBar::GREEN_1 | EightLEDBar::GREEN_2 | EightLEDBar::YELLOW_1 | EightLEDBar::YELLOW_2;
                            }
                            else
                            {
                                if ((percent > 75) && (percent <= 87))
                                {
                                    ledMask = EightLEDBar::BLUE_1 | EightLEDBar::BLUE_2 | EightLEDBar::GREEN_1 | EightLEDBar::GREEN_2 | EightLEDBar::YELLOW_1 | EightLEDBar::YELLOW_2 | EightLEDBar::RED_1;
                                }
                                else
                                {
                                    ledMask = EightLEDBar::BLUE_1 | EightLEDBar::BLUE_2 | EightLEDBar::GREEN_1 | EightLEDBar::GREEN_2 | EightLEDBar::YELLOW_1 | EightLEDBar::YELLOW_2 | EightLEDBar::RED_1 | EightLEDBar::RED_2;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    displayRaw(ledMask);
}
