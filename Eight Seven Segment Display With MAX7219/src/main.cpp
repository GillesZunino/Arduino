#include <Arduino.h>

#include <avr8-stubs-utilities.h>

#include <EightSevenSegmentDisplays.h>


#define SPI_DIN 8
#define SPI_CLK 9
#define SPI_CS 10



EightSevenSegmentDisplays sevenSegmentsDisplays(SPI_DIN, SPI_CS, SPI_CLK);


void allDigitsOn() {
  sevenSegmentsDisplays.clearDisplay();

  sevenSegmentsDisplays.set(0, 0xFF);
  sevenSegmentsDisplays.set(1, 0xFF);
  sevenSegmentsDisplays.set(2, 0xFF);
  sevenSegmentsDisplays.set(3, 0xFF);
  sevenSegmentsDisplays.set(4, 0xFF);
  sevenSegmentsDisplays.set(5, 0xFF);
  sevenSegmentsDisplays.set(6, 0xFF);
  sevenSegmentsDisplays.set(7, 0xFF);
}

void allCharactersForAllDigitsSequential() {
  sevenSegmentsDisplays.clearDisplay();

  constexpr char AllChars[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
      'A' , 'C', 'E', 'F', 'H', 'J', 'L', 'P', 'U', '-', ' '
  };

  for (uint8_t digit = 0; digit < EightSevenSegmentDisplays::NumberOfDigits; digit++) {
    if (digit > 0) {
      sevenSegmentsDisplays.set(digit - 1, 0x00);
    }

    for (uint8_t character = 0; character < sizeof(AllChars) / sizeof(AllChars[0]); character++) {
      sevenSegmentsDisplays.setCharacter(digit, AllChars[character], false);
      delay(500);
      sevenSegmentsDisplays.setCharacter(digit, AllChars[character], true);
      delay(500);
    }

    delay (200);

    if (digit == EightSevenSegmentDisplays::NumberOfDigits - 1) {
      sevenSegmentsDisplays.set(digit, 0x00);
    }
  }
}

void allSegmentsForAllDigitsSequential() {
  sevenSegmentsDisplays.clearDisplay();

  for (uint8_t digit = 0; digit < EightSevenSegmentDisplays::NumberOfDigits; digit++) {
    if (digit > 0) {
      sevenSegmentsDisplays.set(digit - 1, 0x00);
    }
    sevenSegmentsDisplays.set(digit, 0xFF);

    delay (1000);

    if (digit == EightSevenSegmentDisplays::NumberOfDigits - 1) {
      sevenSegmentsDisplays.set(digit, 0x00);
    }
  }
}

void decimalPointsChase() {
  sevenSegmentsDisplays.clearDisplay();

  for (uint8_t digit = 0; digit < EightSevenSegmentDisplays::NumberOfDigits; digit++) {
    sevenSegmentsDisplays.setDecimalPoint(digit, true);
    delay(500);
  }

  for (int8_t digit = EightSevenSegmentDisplays::NumberOfDigits - 1; digit >= 0; digit--) {
    sevenSegmentsDisplays.setDecimalPoint(digit, false);
    delay(500);
  }
}

void bounceString(const char string[]) {
  uint8_t length = strlen(string);
  if (length < EightSevenSegmentDisplays::NumberOfDigits) {
    sevenSegmentsDisplays.clearDisplay();

    for (int8_t index = 0; index <= EightSevenSegmentDisplays::NumberOfDigits - length; index++) {
      for (int8_t blankIndex = 0; blankIndex < index; blankIndex++) {
        sevenSegmentsDisplays.setCharacter(blankIndex, ' ', false);
      }

      for (int8_t charPos = 0; charPos < length; charPos++) {
        sevenSegmentsDisplays.setCharacter(index + charPos, string[charPos], false);
      }

      delay (500);
    }

    delay (200);

    for (int8_t index = EightSevenSegmentDisplays::NumberOfDigits - length; index >=0; index--) {
      for (int8_t blankIndex = EightSevenSegmentDisplays::NumberOfDigits - 1; blankIndex > index; blankIndex--) {
        sevenSegmentsDisplays.setCharacter(blankIndex, ' ', false);
      }

      for (int8_t charPos = 0; charPos < length; charPos++) {
        sevenSegmentsDisplays.setCharacter(index + charPos, string[charPos], false);
      }

      delay (500);
    }
  }
}



void setup() {
  INIT_DEBUGGER();

  sevenSegmentsDisplays.initialize();
  sevenSegmentsDisplays.shutdown(false);

  sevenSegmentsDisplays.clearDisplay();
}

void loop() {
  // Built in test mode
  sevenSegmentsDisplays.displayTest(true);
  delay(5000);
  sevenSegmentsDisplays.displayTest(false);

  // Configure for lower brightness
  sevenSegmentsDisplays.setBrightness(4);

  // All on
  allDigitsOn();
  delay(5000);

  // Display all characters on all digits sequentially
  allCharactersForAllDigitsSequential();

  // Enable all segments for all digits from left to right
  allSegmentsForAllDigitsSequential();

  // Enable all decimal points and then disable them in reverse
  decimalPointsChase();

  // Bounce the given string from left to right
  bounceString("HELP");
  delay(500);
}