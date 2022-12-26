#include <Arduino.h>

#include "DHTStable.h"


#if DEBUG
#include "avr8-stub.h"
#include "app_api.h"
#endif

#define ULTRASONIC_SENSOR_TRIGER_PIN 9
#define ULTRASONIC_SENSOR_ECHO_PIN 8


#define DHT11_DATA_PIN 4


void readDistance(float soundSpeedInMetersPerSecond, float& distanceInCm, unsigned long& echoDurationInMicroseconds);
float adjustSpeedOfSoundToCurrentConditions();
bool getCurrentTemperatureAndRelativeHumidity(float& temperature, float& relativeHumidity);


void setup() {
#if DEBUG
  // Initialize GDB stub - The debugger uses INT0 [pin 2 (Uno) or pin 21 (Mega)] so DO NOT USE these pins !
  debug_init();
#endif

  pinMode(ULTRASONIC_SENSOR_TRIGER_PIN, OUTPUT);
  pinMode(ULTRASONIC_SENSOR_ECHO_PIN, INPUT);

  Serial.begin(115200);
}

void loop() {
  // Get an adjusted speed of sound in air
  float temperatureCorrectedSoundSpeedInMeterPerSecond = adjustSpeedOfSoundToCurrentConditions();

  unsigned long echoDurationInMicroseconds;
  float distanceInCm;
  readDistance(temperatureCorrectedSoundSpeedInMeterPerSecond, distanceInCm, echoDurationInMicroseconds);

  String s = "T:" + String(echoDurationInMicroseconds) + "  D:" + String(distanceInCm) + "cm [" + String(distanceInCm * 0.39370079) + "in]\n";
  Serial.write(s.c_str());

  // The sensor has a 50 micro seconds 'dead time' between bursts
  delay(500);
}

void readDistance(float soundSpeedInMetersPerSecond, float& distanceInCm, unsigned long& echoDurationInMicroseconds) {
  // Ensure the trigger is LOW
  digitalWrite(ULTRASONIC_SENSOR_TRIGER_PIN, LOW);
  delayMicroseconds(4);

  // Trigger 40kHz Ultrasonic burst
  digitalWrite(ULTRASONIC_SENSOR_TRIGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_SENSOR_TRIGER_PIN, LOW);
  delayMicroseconds(10);

  // Calculate length of the echo pulse
  const unsigned long PulseLengthTimeout = 150000L;
  unsigned long durationInMicroSeconds = pulseIn(ULTRASONIC_SENSOR_ECHO_PIN, HIGH, PulseLengthTimeout);
  echoDurationInMicroseconds = durationInMicroSeconds;

  // Calculate distance in cm
  distanceInCm = (durationInMicroSeconds / 20000.0) * soundSpeedInMetersPerSecond;
}

float adjustSpeedOfSoundToCurrentConditions() {
  const unsigned long AdjustSoundSpeedEveryInMs = 30 * 1000;

  static unsigned long lastAdjustmentMilliseconds = 0;
  static float temperatureCorrectedSoundSpeedInMeterPerSecond = 0;

  if ((lastAdjustmentMilliseconds == 0) || (millis() - lastAdjustmentMilliseconds > AdjustSoundSpeedEveryInMs)) {
    float temperature;
    float relativeHumidity;
    bool hasTemperatureAndHumidityFromSensor = getCurrentTemperatureAndRelativeHumidity(temperature, relativeHumidity);
    if ((temperatureCorrectedSoundSpeedInMeterPerSecond == 0.0) || hasTemperatureAndHumidityFromSensor) {
      // TODO: Account for humidity
      temperatureCorrectedSoundSpeedInMeterPerSecond = 331.3 + (0.606 * temperature);
      lastAdjustmentMilliseconds = millis();

      String s = "Adjusted Sound Speed to " + String(temperatureCorrectedSoundSpeedInMeterPerSecond) + "m/s " + String(hasTemperatureAndHumidityFromSensor ? "[FROM SENSOR]" : "[NO SENSOR]") + " [T:" + String(temperature) + "C/H:" + String(relativeHumidity) + "%]\n";
      Serial.write(s.c_str());
    }
  }

  return temperatureCorrectedSoundSpeedInMeterPerSecond;
}

bool getCurrentTemperatureAndRelativeHumidity(float& temperature, float& relativeHumidity) {

  static DHTStable dht11;

  int chk = dht11.read11(DHT11_DATA_PIN);
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