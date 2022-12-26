#include <Arduino.h>

#if DEBUG
#include "avr8-stub.h"
#include "app_api.h"
#endif

#include <EightLEDBar.h>



#define LED_BAR__DS_PIN 12    // "SERIAL DATA" - To shift register pin 'DS'    (14)
#define LED_BAR_SH_CP_PIN 9   // "CLOCK"       - To shift register pin 'SH_CP' (11)
#define LEDBAR_ST_CP_PIN 11   // "LATCH"       - To shift register pin 'ST_CP' (12)

EightLEDBar ledBarDriver(LED_BAR__DS_PIN, LED_BAR_SH_CP_PIN, LEDBAR_ST_CP_PIN);


#define PHOTORESISTOR_INPUT_PIN A0



void upAndDown();
void singleLedForwardLoop();
void displayLightIntensity();



void setup()
{
#if DEBUG
  // Initialize GDB stub - The debugger uses INT0 [pin 2 (Uno) or pin 21 (Mega)] so DO NOT USE these pins !
  debug_init();
#endif

  // Configure input to read the photo resistor value
  pinMode(PHOTORESISTOR_INPUT_PIN, INPUT);

#if DEBUG
  Serial.begin(115200);
  while (!Serial) { }
#endif
}

void loop() 
{
  displayLightIntensity();

  //upAndDown();
  //delay (200);

  //singleLedForwardLoop();
  //delay (1000);
}

void upAndDown()
{
  const unsigned long pauseInMilliseconds = 100;

  uint8_t leds = 0;

  for (int i = 0; i < EightLEDBar::LEDCount; i++)
  {
    bitSet(leds, i);
    ledBarDriver.displayRaw(leds);
    delay(pauseInMilliseconds);
  }

  delay(pauseInMilliseconds);

  for (int i = EightLEDBar::LEDCount - 1; i >= 0; i--)
  {
    bitClear(leds, i);
    ledBarDriver.displayRaw(leds);
    delay(pauseInMilliseconds);
  }
}

void singleLedForwardLoop()
{
  const unsigned long pauseInMilliseconds = 200;

  ledBarDriver.displayRaw(EightLEDBar::BLUE_1);
  delay(pauseInMilliseconds);

  ledBarDriver.displayRaw(EightLEDBar::BLUE_2);
  delay(pauseInMilliseconds);

  ledBarDriver.displayRaw(EightLEDBar::GREEN_1);
  delay(pauseInMilliseconds);

  ledBarDriver.displayRaw(EightLEDBar::GREEN_2);
  delay(pauseInMilliseconds);

  ledBarDriver.displayRaw(EightLEDBar::YELLOW_1);
  delay(pauseInMilliseconds);

  ledBarDriver.displayRaw(EightLEDBar::YELLOW_2);
  delay(pauseInMilliseconds);

  ledBarDriver.displayRaw(EightLEDBar::RED_1);
  delay(pauseInMilliseconds);

  ledBarDriver.displayRaw(EightLEDBar::RED_2);
  delay(pauseInMilliseconds);

  ledBarDriver.displayRaw(EightLEDBar::NONE);
}


void displayLightIntensity()
{
  // Read photo resistor value
  static int previousReading = 0;
  int reading = analogRead(PHOTORESISTOR_INPUT_PIN);
  if (reading != previousReading)
  {
    previousReading = reading;

    // === Photoresistor KLS6-3537 ====================================================
    // * 0  lux (dark)  -> 2M Ohm              [Measured 3.2 M Ohm]
    // * 10 lux (light) -> 18K Ohm to 50 K Ohm [Measured 200 Ohm when illuminated]
    // 
    //  Vcc --- Photoresistor ---- 10K Ohm ---- GND
    //                          |
    //                        Pin A0
    //
    // We measure across a 10K Ohm resistor. The photoresistor is supplied with 5V
    // Using a voltage divider: VAO = (5V * 10KOhm) / (10K Ohm + RPhotoresistor)
    // ================================================================================

    // Arduino Analog to Digital resolution is 10 bits
    const int MaxValueAnalogRead = 1023;

#if DEBUG
    // Calculate the observed voltage
    const float Vcc = 4.5;
    float voltage = (Vcc * reading) / MaxValueAnalogRead;
#endif

    // Calculate the percentage of the LED Bar to light up - Dark: 0% and Full lit: 100%
    uint8_t lightIntensityInPercent = (100.0 * reading) / MaxValueAnalogRead;
    ledBarDriver.displayPercent(lightIntensityInPercent);

#if DEBUG
    String serialOutput;

    serialOutput.concat("R:");
    serialOutput.concat(reading);
    serialOutput.concat("-V:");
    serialOutput.concat(voltage);
    serialOutput.concat("-P:");
    serialOutput.concat(lightIntensityInPercent);

    Serial.println(serialOutput);
#endif
  }

  // Ensure we only poll slower than the fastest refresh = Our photoresistor takes about 30ms to react to change
  const unsigned long timeBetweenReadingInMilliseconds = 50;
  delay(timeBetweenReadingInMilliseconds);
}