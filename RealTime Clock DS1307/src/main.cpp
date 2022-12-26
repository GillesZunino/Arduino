#include <SPI.h>
#include "RTClib.h"


#if DEBUG
#include "avr8-stub.h"
#endif



RTC_DS1307 clock;


// ========================================================================
// DS1307 Connections
//   Sensor  Arduino
//    GND -> GND
//    Vcc -> Vcc
//    SDA -> 20 (SDA) [Mega 2560] - A4 [Nano]
//    SCL -> 21 (SCL) [Mega 2560] - A5 [Nano]
//    SQW -> Not Connected
// ========================================================================



void displayClockMode() {
  Ds1307SqwPinMode mode = clock.readSqwPinMode();
  Serial.print("Sqw Pin Mode: ");
  switch (mode) {
    case DS1307_OFF:              Serial.println("OFF");       break;
    case DS1307_ON:               Serial.println("ON");        break;
    case DS1307_SquareWave1HZ:    Serial.println("1Hz");       break;
    case DS1307_SquareWave4kHz:   Serial.println("4.096kHz");  break;
    case DS1307_SquareWave8kHz:   Serial.println("8.192kHz");  break;
    case DS1307_SquareWave32kHz:  Serial.println("32.768kHz"); break;
    default:                      Serial.println("UNKNOWN");   break;
  }
}

void setup()
{
  #if DEBUG
  // Initialize GDB stub
  debug_init();
#endif

  Serial.begin(115200);

  Serial.println();
  Serial.println("Initialize RTC module");

  if (!clock.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (!clock.isrunning())
  {
    Serial.println("RTC is NOT running, let's set the time!");

    // Manual Clock Set (YYYY, MM, DD, HH, II, SS)
    //clock.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  
    // Can configure the clock to the Date / Time the sketch was compiled
    //clock.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  else
  {
    Serial.println("RTC is running. Date / Time not updated");
  }
  
  displayClockMode();

  Serial.println();
}

void loop()
{
  DateTime dt = clock.now();

  char formattedDate[255] = "DDD MM-DD-YYYY hh:mm:ss";
  dt.toString(formattedDate);
  Serial.println(formattedDate);

  delay(1000);
}
