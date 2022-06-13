#include <Arduino.h>
#include <Wire.h>


#if DEBUG
#include "avr8-stub.h"
#include "app_api.h"
#endif


#include <LiquidCrystal.h>
#include <DHTStable.h>



#define LCD_RS_PIN 6
#define LCD_E_PIN 8
#define LCD_RW_PIN 7

#define LCS_D4_PIN 12
#define LCS_D5_PIN 11
#define LCS_D6_PIN 10
#define LCS_D7_PIN 9


#define DHT11_DATA_PIN 4

#define PHOTORESISTOR_INPUT_PIN A0



bool getCurrentTemperatureAndRelativeHumidity(float& temperature, float& relativeHumidity, uint8_t dht11Pin);
void getCurrentLightIntensity(float& voltage, uint8_t& lightIntensityInPercent, uint8_t photoResistorPin);



// When in 4 bits mode, data is sent to D3, D4, D5, D6
LiquidCrystal  lcd(LCD_RS_PIN, LCD_RW_PIN, LCD_E_PIN, LCS_D4_PIN, LCS_D5_PIN, LCS_D6_PIN, LCS_D7_PIN);


void setup() {
#if DEBUG
  // Initialize GDB stub - The debugger uses INT0 [pin 2 (Uno) or pin 21 (Mega)] so DO NOT USE these pins !
  debug_init();
#endif

  lcd.begin(16, 2, LCD_5x8DOTS);
  lcd.clear();

  // Configure input to read DHT11 data
  pinMode(DHT11_DATA_PIN, INPUT);

  // Configure input to read the photo resistor value
  pinMode(PHOTORESISTOR_INPUT_PIN, INPUT);
}

void loop() {
    // Temperature and Humidity
    float temperature;
    float relativeHumidity;
    bool hasTemperatureAndHumidityFromSensor = getCurrentTemperatureAndRelativeHumidity(temperature, relativeHumidity, DHT11_DATA_PIN);
    String temperatureAndHumidityString = String(hasTemperatureAndHumidityFromSensor ? "S " : "N ") + String(temperature) + "C-" + String(relativeHumidity) + "%";
    
    // Current light intensity
    float voltage;
    uint8_t lightIntensityInPercent;
    getCurrentLightIntensity(voltage, lightIntensityInPercent, PHOTORESISTOR_INPUT_PIN);
    String lightIntensity = "V:" + String(voltage) + "V [" + String(lightIntensityInPercent) + "%]";


    // LCD display
    lcd.clear();

    // * - Temperature and humidity
    lcd.setCursor(0, 0);
    lcd.print(temperatureAndHumidityString);
   
    // * - Light intensity
    lcd.setCursor(1, 1);
    lcd.print(lightIntensity);

    delay (2000);
}

bool getCurrentTemperatureAndRelativeHumidity(float& temperature, float& relativeHumidity, uint8_t dht11Pin) {

  static DHTStable dht11;

  int chk = dht11.read11(dht11Pin);
  switch (chk)
  {
    case DHTLIB_OK:
      temperature = dht11.getTemperature();
      relativeHumidity = dht11.getHumidity();
      return true;

    case DHTLIB_ERROR_CHECKSUM:
    case DHTLIB_ERROR_TIMEOUT:
    default:
      // Default if no temperature / humidity sensor is connected
      temperature = 19.0;
      relativeHumidity = 0.0;
      return false;
  }
}

void getCurrentLightIntensity(float& voltage, uint8_t& lightIntensityInPercent, uint8_t photoResistorPin)
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

    // Calculate the observed voltage
    const float Vcc = 4.5;
    voltage = (Vcc * reading) / MaxValueAnalogRead;

    // Calculate the percentage of the LED Bar to light up - Dark: 0% and Full lit: 100%
    lightIntensityInPercent = (100.0 * reading) / MaxValueAnalogRead;
  }
}