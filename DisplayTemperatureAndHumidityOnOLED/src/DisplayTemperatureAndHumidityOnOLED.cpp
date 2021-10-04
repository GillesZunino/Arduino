#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include "DHTStable.h"


#if DEBUG
#include "avr8-stub.h"
#endif


// OLED Constants =====================================================================================
//
// SPI Wiring for OLED Display Pins on Arduino Nano
//
//  SPI   |  Display Name  | Wire color | Uno 'D' Pin | User Chosen
//------------------------------------------------------------------
//  SCLK  |       CLK      |    Yellow  |      D13    |    No (Hard Wired)
//  MOSI  |       DIN      |     Blue   |      D11    |    No (Hard Wired)
//  MISO  |        -       |            |             |
//   SS   |       CS       |    Orange  |      D10    |    No (Hard Wired)
//   DC   |       DC       |    Green   |      D9     |    Yes
//  RST   |       RST      |    White   |      D8     |    Yes
//

#define OLED_CS_D_PIN  10 // Nano defines SS (Slave Select) aka CS (Chip Select) as D10

#define OLED_DC_D_PIN  9
#define OLED_RST_D_PIN 8


// DHT11 constants ====================================================================================
#define DHT11_D_PIN 3


// Visual Layout constants ============================================================================
// Space between an axis line and axis legend
const int16_t TextMarginXPixel = 4;
const int16_t TextLeadingPixel = 6;

// Temperature and Humidity axis X Pixel 
const int16_t TemperatureScaleXPixel = 20;
const int16_t HumidityScaleXPixel = 128 - 28;

// Graph size including axis
const uint16_t GraphHeightPixel = 111;
const uint16_t GraphWidthPixel = HumidityScaleXPixel - TemperatureScaleXPixel;

// Graph to left location
const uint16_t GraphTopPixel = 0;
const uint16_t GraphLeftPixel = TemperatureScaleXPixel + 2;

// Graph usable size (excludes axis)
const uint16_t GraphUsableWidthPixel = GraphWidthPixel - 2;
const uint16_t GraphUsableHeightPixel = GraphHeightPixel - 1;

// Vertical location (Y) of the summary section
const int16_t SummarySectionStartYPixel = 120;


// Temperature auto scale constants ===================================================================
const uint8_t InitialTemperatureRangeDisplayed = 15;
const uint8_t TemperatureAutoscaleRange = 5;


// Prototypes =========================================================================================
bool readTemperatureAndHumidityFromSensor(float &temperature, float &humidity);
bool updateTemperatureScale(float temperature, const uint8_t temperatureDataPoints[], int dataPointsCount, uint8_t &scaleMinTemperature, uint8_t& scaleMaxTemperature);
void appendNewMeasurements(float temperature, float humidity, uint8_t temperatureDataPoints[], uint8_t humidityDataPoints[], uint8_t& dataPointsCount);

void drawTemperatureAndHumidityCurves(const uint8_t temperatureDataPoints[], const uint8_t humidityDataPoints[], uint8_t dataPointsCount, uint8_t temperatureScaleMin, uint8_t temperatureScaleMax);
void drawTemperatureAndHumiditySummary(float temperature, float humidity);
void drawDynamicTemperatureScale(int8_t scaleMinTemperature, int8_t scaleMaxTemperature);
void drawCanvasLegend(void);



// Local variables =====================================================================================
Arduino_DataBus *bus = new Arduino_HWSPI(OLED_DC_D_PIN, OLED_CS_D_PIN);
Arduino_GFX *gfx = new Arduino_SSD1351(bus, OLED_RST_D_PIN, 0 /* rotation */);

DHTStable dht11;

// DHT11 has Temperature Range: 0°C to 50°C with integral resolution - Humidity Range: 20% to 90% with integral resolution so we can use uint8_t
const uint8_t NumberOfHistoricMeasurements = GraphUsableWidthPixel;

uint8_t temperatureScaleMin = 0, temperatureScaleMax = 0;
uint8_t historicTemperature[NumberOfHistoricMeasurements];
uint8_t historicHumidity[NumberOfHistoricMeasurements];
uint8_t dataPointsCollected = 0;



void setup()
{
#if DEBUG
  // Initialize GDB stub
  debug_init();
#endif

    gfx->begin();
    gfx->fillScreen(BLACK);

    drawCanvasLegend();
}

void loop()
{
    float temperature, humidity;
    if (readTemperatureAndHumidityFromSensor(temperature, humidity))
    {
        // Read temperature and humidity from sensor, calculate temperature auto scale and add measurements to the list of historic points
        bool temperatureScaleChanged = updateTemperatureScale(temperature, historicTemperature, dataPointsCollected, temperatureScaleMin, temperatureScaleMax);
        appendNewMeasurements(temperature, humidity, historicTemperature, historicHumidity, dataPointsCollected);
        if (temperatureScaleChanged) { drawDynamicTemperatureScale(temperatureScaleMin, temperatureScaleMax); }
        
        // Draw historic curves and update measurements summary
        drawTemperatureAndHumidityCurves(historicTemperature, historicHumidity, dataPointsCollected, temperatureScaleMin, temperatureScaleMax);
        drawTemperatureAndHumiditySummary(temperature, humidity);
    }

    // DHT11 can only acquire measurements at a rate of one every 2s
    delay(2000);
}

bool readTemperatureAndHumidityFromSensor(float &temperature, float &humidity)
{
    int chk = dht11.read11(DHT11_D_PIN);
    switch (chk)
    {
      case DHTLIB_OK:
        temperature = dht11.getTemperature();
        humidity = dht11.getHumidity();
        return true;

      case DHTLIB_ERROR_CHECKSUM:
      case DHTLIB_ERROR_TIMEOUT:
      default:
        temperature = 0.0f;
        humidity = 0.0f;
        return false;
    }
}


bool updateTemperatureScale(float temperature, const uint8_t temperatureDataPoints[], int dataPointsCount, uint8_t &scaleMinTemperature, uint8_t& scaleMaxTemperature)
{
    // Initially we do not have a scale or previous data points
    if (dataPointsCount == 0)
    {
        scaleMaxTemperature = (uint8_t) (temperature + (InitialTemperatureRangeDisplayed / 2.0));
        scaleMinTemperature = (uint8_t) (temperature - (InitialTemperatureRangeDisplayed / 2.0));
        return true;
    }
    else
    {
        // Get the scale based on the min / max of currently known data points
        if ((temperature <= scaleMinTemperature) || (temperature >= scaleMaxTemperature))
        {
            uint8_t minFound = temperatureDataPoints[0],  maxFound = temperatureDataPoints[0];
            for (int temperatureIndex = 1; temperatureIndex < dataPointsCount; temperatureIndex++)
            {
                if (temperatureDataPoints[temperatureIndex] > maxFound) { maxFound = temperatureDataPoints[temperatureIndex]; }
                if (temperatureDataPoints[temperatureIndex] < minFound) { minFound = temperatureDataPoints[temperatureIndex]; }
            }

            scaleMaxTemperature = maxFound + TemperatureAutoscaleRange;
            scaleMinTemperature = minFound - TemperatureAutoscaleRange;

            return true;
        }
    }

    return false;
}

void appendNewMeasurements(float temperature, float humidity, uint8_t temperatureDataPoints[], uint8_t humidityDataPoints[], uint8_t& dataPointsCount)
{
  if (dataPointsCount < NumberOfHistoricMeasurements)
  {
      temperatureDataPoints[dataPointsCount] = temperature;
      humidityDataPoints[dataPointsCount] = humidity;
      dataPointsCount++;
  }
  else 
  {
      memmove(temperatureDataPoints, temperatureDataPoints + 1, NumberOfHistoricMeasurements - 1);
      memmove(humidityDataPoints, humidityDataPoints + 1, NumberOfHistoricMeasurements - 1);

      temperatureDataPoints[NumberOfHistoricMeasurements - 1] = temperature;
      humidityDataPoints[NumberOfHistoricMeasurements - 1] = humidity;
  }
}


void drawTemperatureAndHumidityCurves(const uint8_t temperatureDataPoints[], const uint8_t humidityDataPoints[], uint8_t dataPointsCount, uint8_t temperatureScaleMin, uint8_t temperatureScaleMax)
{
    gfx->startWrite();

      gfx->writeFillRect(GraphLeftPixel, GraphTopPixel, GraphUsableWidthPixel, GraphUsableHeightPixel, BLACK);

      uint8_t previousX, previousY;
      for (uint8_t temperatureIndex = 0; temperatureIndex < dataPointsCount; temperatureIndex++)
      {
          uint8_t temperature = temperatureDataPoints[temperatureIndex];
          uint8_t temperaturePixel = GraphUsableHeightPixel - (uint8_t) (((temperature - temperatureScaleMin) * (float) GraphUsableHeightPixel) / (temperatureScaleMax - temperatureScaleMin));
          uint8_t xAxisPixel = GraphLeftPixel + temperatureIndex;
          if (temperatureIndex == 0)
          {
              gfx->writePixel(xAxisPixel, temperaturePixel, BLUE);
          }
          else
          {
              gfx->writeLine(previousX, previousY, xAxisPixel, temperaturePixel, BLUE);
          }

          previousX = xAxisPixel;
          previousY = temperaturePixel;
      }

      for (uint8_t humidityIndex = 0; humidityIndex < dataPointsCount; humidityIndex++)
      {
          uint8_t humidity = humidityDataPoints[humidityIndex];
          uint8_t humidityPixel = GraphUsableHeightPixel - (uint8_t) ((humidity * (float) GraphUsableHeightPixel) / 100.0f);
          uint8_t xAxisPixel = GraphLeftPixel + humidityIndex;
          if (humidityIndex == 0)
          {
              gfx->writePixel(xAxisPixel, humidityPixel, YELLOW);
          }
          else
          {
              gfx->writeLine(previousX, previousY, xAxisPixel, humidityPixel, YELLOW);
          }

          previousX = xAxisPixel;
          previousY = humidityPixel;
      }

    gfx->endWrite();
}

void drawDynamicTemperatureScale(int8_t scaleMinTemperature, int8_t scaleMaxTemperature)
{
    gfx->setTextColor(BLUE, BLACK);
    gfx->setCursor(0, 0);

    char temperatureString[] = "00C";
    dtostrf(scaleMaxTemperature, 2, 0, temperatureString);
    temperatureString[2] = 'C';
    gfx->print(temperatureString);

    dtostrf(scaleMinTemperature, 2, 0, temperatureString);
    temperatureString[2] = 'C';
    gfx->setCursor(0, GraphHeightPixel - TextLeadingPixel);
    gfx->print(temperatureString);
}

void drawCanvasLegend()
{
    gfx->setTextColor(YELLOW, BLACK);
    gfx->setCursor(HumidityScaleXPixel + TextMarginXPixel, 0);
    gfx->print("100%");

    gfx->setCursor(HumidityScaleXPixel + TextMarginXPixel, GraphHeightPixel - TextLeadingPixel);
    gfx->print("0%");

    gfx->drawFastVLine(TemperatureScaleXPixel, 0, GraphHeightPixel, BLUE);
    gfx->drawFastVLine(HumidityScaleXPixel, 0, GraphHeightPixel, YELLOW);
    gfx->drawFastHLine(TemperatureScaleXPixel, GraphHeightPixel, GraphWidthPixel, GREEN);
}

void drawTemperatureAndHumiditySummary(float temperature, float humidity)
{
    const int16_t TemperatureXPixel = 34;
    const int16_t SpacingXPixel = 34;

    gfx->setTextColor(BLUE, BLACK);
    gfx->setCursor(TemperatureXPixel, SummarySectionStartYPixel);

    char temperatureString[] = "00C";
    dtostrf(temperature, 2, 0, temperatureString);
    temperatureString[2] = 'C';
    gfx->print(temperatureString);

    gfx->setTextColor(YELLOW, BLACK);
    gfx->setCursor(TemperatureXPixel + SpacingXPixel, SummarySectionStartYPixel);

    char humidityString[] = "00%";
    dtostrf(humidity, 2, 0, humidityString);
    humidityString[2] = '%';
    gfx->print(humidityString);
}
