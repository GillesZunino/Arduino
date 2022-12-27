#include <Arduino.h>

#include <Adafruit_NeoPixel.h>

#include <avr8-stubs-utilities.h>


#define LEDSTRIP_DIN_PIN 6


constexpr int PixelCount = 144;
constexpr int LedBrightness = 50;


const uint32_t Red = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t Green = Adafruit_NeoPixel::Color(0, 255, 0);
const uint32_t Blue = Adafruit_NeoPixel::Color(0, 0, 255);
const uint32_t White = Adafruit_NeoPixel::Color(255, 255, 255);
const uint32_t Black = Adafruit_NeoPixel::Color(0, 0, 0);


//
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
//
Adafruit_NeoPixel ledStrip = Adafruit_NeoPixel(PixelCount, LEDSTRIP_DIN_PIN, NEO_GRB);


// ============================================================================================================================

void AllLit(uint32_t color, Adafruit_NeoPixel& ledStrip) {
  for(uint16_t i = 0; i < ledStrip.numPixels(); i++) {
    ledStrip.setPixelColor(i, color);
  }

  ledStrip.show();
}

void FirstLit(uint32_t color, Adafruit_NeoPixel& ledStrip) {
  ledStrip.setPixelColor(0, color);

  for(uint16_t i = 1; i < ledStrip.numPixels(); i++) {
    ledStrip.setPixelColor(i, Black);
  }

  ledStrip.show();
}

void AllOff(Adafruit_NeoPixel& ledStrip) {
  ledStrip.clear();
  ledStrip.show();
}

// Fill the dots one after the other with a color
void ColorWipe(uint32_t color, uint8_t wait, Adafruit_NeoPixel& ledStrip) {
  for(uint16_t i = 0; i < ledStrip.numPixels(); i++) {
    ledStrip.setPixelColor(i, color);
    ledStrip.show();
    delay(wait);
  }
}

// Theatre-style crawling lights
void TheaterChase(uint32_t color, uint8_t wait, Adafruit_NeoPixel& ledStrip) {
  for (int j = 0; j < 10; j++) {  //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < ledStrip.numPixels(); i = i + 3) {
        ledStrip.setPixelColor(i+q, color);
      }
      ledStrip.show();

      delay(wait);

      for (uint16_t i = 0; i < ledStrip.numPixels(); i = i + 3) {
        ledStrip.setPixelColor(i+q, 0);
      }
    }
  }
}

// Given a color wheelPos transition colors from r - g - b - back to r
uint32_t Wheel(byte wheelPos) {
  wheelPos = 255 - wheelPos;
  if (wheelPos < 85) {
    return ledStrip.Color(255 - wheelPos * 3, 0, wheelPos * 3);
  }
  if (wheelPos < 170) {
    wheelPos -= 85;
    return ledStrip.Color(0, wheelPos * 3, 255 - wheelPos * 3);
  }
  wheelPos -= 170;
  return ledStrip.Color(wheelPos * 3, 255 - wheelPos * 3, 0);
}

// Theatre-style crawling lights with rainbow effect
void TheaterChaseRainbow(uint8_t wait, Adafruit_NeoPixel& ledStrip) {
  for (int j = 0; j < 256; j++) {
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < ledStrip.numPixels(); i = i + 3) {
        ledStrip.setPixelColor(i+q, Wheel( (i+j) % 255));
      }
      ledStrip.show();

      delay(wait);

      for (uint16_t i = 0; i < ledStrip.numPixels(); i = i + 3) {
        ledStrip.setPixelColor(i+q, 0);
      }
    }
  }
}

// Rainbow effect
void Rainbow(uint8_t wait, Adafruit_NeoPixel& ledStrip) {
  uint16_t i, j;

  for(j = 0; j < 256; j++) {
    for(i = 0; i < ledStrip.numPixels(); i++) {
      ledStrip.setPixelColor(i, Wheel((i+j) & 255));
    }
    ledStrip.show();
    delay(wait);
  }
}

// Rainbow, with colors equally distributed throughout
void RainbowCycle(uint8_t wait, Adafruit_NeoPixel& ledStrip) {
  uint16_t i, j;

  for(j = 0; j < 256 * 5; j++) {
    for(i = 0; i< ledStrip.numPixels(); i++) {
      ledStrip.setPixelColor(i, Wheel(((i * 256 / ledStrip.numPixels()) + j) & 255));
    }
    ledStrip.show();
    delay(wait);
  }
}

// ============================================================================================================================



void setup() {
  INIT_DEBUGGER();

  ledStrip.begin();
  ledStrip.setBrightness(LedBrightness);
  ledStrip.show();
}

void loop() {
  // First LED - Solid colors
  FirstLit(White, ledStrip);
  FirstLit(Red, ledStrip);
  FirstLit(Green, ledStrip);
  FirstLit(Blue, ledStrip);

  // All LEDs - Solid colors
  AllLit(White, ledStrip);
  AllLit(Red, ledStrip);
  AllLit(Green, ledStrip);
  AllLit(Blue, ledStrip);

  // ALl off
  AllOff(ledStrip);

  // Wipes
  ColorWipe(Red, 50, ledStrip);
  ColorWipe(Green, 50, ledStrip);
  ColorWipe(Blue, 50, ledStrip);

  // Theater Chases
  TheaterChase(ledStrip.Color(127, 127, 127), 50, ledStrip);
  TheaterChase(ledStrip.Color(127, 0, 0), 50, ledStrip);
  TheaterChase(ledStrip.Color(0, 0, 127), 50, ledStrip);
  TheaterChaseRainbow(50, ledStrip);

  // Rainbows
  Rainbow(20, ledStrip);
  RainbowCycle(20, ledStrip);
}
