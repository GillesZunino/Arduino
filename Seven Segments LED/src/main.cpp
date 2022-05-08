#include <Arduino.h>

#if DEBUG
#include "avr8-stub.h"
#include "app_api.h"
#endif

#include <SingleSevenSegmentsDisplay.h>



#define BUITLIN_LED_PIN 13

#define SEVEN_SEGMENTS_DS_PIN 5      // "SERIAL DATA" - To shift register pin 'DS'    (14)
#define SEVEN_SEGMENTS_SH_CP_PIN 3   // "CLOCK"       - To shift register pin 'SH_CP' (11)
#define SEVEN_SEGMENTS_ST_CP_PIN 4   // "LATCH"       - To shift register pin 'ST_CP' (12)



SingleSevenSegmentsDisplay singleSevenSegmentDisplay(SEVEN_SEGMENTS_DS_PIN, SEVEN_SEGMENTS_SH_CP_PIN, SEVEN_SEGMENTS_ST_CP_PIN);


void setup() {
#if DEBUG
  // Initialize GDB stub - The debugger uses INT0 [pin 2 (Uno) or pin 21 (Mega)] so DO NOT USE these pins !
  debug_init();
#endif

  // Configure BUITLIN_LED_PIN and turn the built in LED on to indicate we are initializing
  pinMode(BUITLIN_LED_PIN, OUTPUT);
  digitalWrite(BUITLIN_LED_PIN, HIGH);
}

void loop() {
  for (int currentDigit = 0; currentDigit <= 9 ; currentDigit++)
  {
    singleSevenSegmentDisplay.displayDigit(currentDigit, true);
    delay(1000);
  }

  singleSevenSegmentDisplay.displayLetter('A', true);
  delay(1000);

  singleSevenSegmentDisplay.displayLetter('C', true);
  delay(1000);

  singleSevenSegmentDisplay.displayLetter('E', true);
  delay(1000);

  singleSevenSegmentDisplay.displayLetter('F', true);
  delay(1000);

  singleSevenSegmentDisplay.displayLetter('H', true);
  delay(1000);

  singleSevenSegmentDisplay.displayLetter('J', true);
  delay(1000);

  singleSevenSegmentDisplay.displayLetter('L', true);
  delay(1000);

  singleSevenSegmentDisplay.displayLetter('P', true);
  delay(1000);

  singleSevenSegmentDisplay.displayLetter('U', true);
  delay(1000);
}
