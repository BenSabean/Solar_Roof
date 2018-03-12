#include <OneWire.h>
#include <DallasTemperature.h>
#include <Smooth.h>

// Sensor counts
#define ANALOG_SEN    11          // Number of analog sensor connected 10 Current 1 Voltage 1 Temp
#define TOTAL_SEN     12          // Total number of sensors
// Delays
#define READING_MS    5           // Delay in ms between readings 
                                  // Total Readings delay = (READING_MS * TOTAL_SEN * READINGS) 
#define SERIAL_MS     200         // Delay between serial messages
                                  // Total Serial delay = (SERIAL_MS * TOTAL_SEN)
// Sensor callibration
#define READINGS      200         // Average every reading value (for ~ 1min readings = 800)

// One Wire setup
#define ONE_WIRE      3           // Pin for connecting OneWire sensors
OneWire oneWire(ONE_WIRE);
DallasTemperature Temperature(&oneWire);

// Sensor Pins
int SEN[ANALOG_SEN] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10};
// Buffers for averaging the readings
Smooth s[TOTAL_SEN];

// Calculated slope and offset -> digital = [slope]*voltage + [offset]
float Slope[] = {
  20.516,    // S0
  20.557,    // S1
  19.894,    // S2
  20.042,    // S3
  19.978,    // S4
  20.028,    // S5
  20.377,    // S6
  20.367,    // S7
  20.142,    // S8
  20.54,    // S9
  1     // S10
};
float Offset[] = {
  577,    // S0 576.41
  578,    // S1 577.47
  571,    // S2 570.38
  576,    // S3 576.3
  576,    // S4 575.6
  575,    // S5 573.24
  579,    // S6 576.83
  575,    // S7 574.63
  573,    // S8 571.2
  575,    // S9 574.83
  0     // S10
};

void setup()
{
  // Serial for Monitoring
  Serial.begin(115200);
  Serial.println(" -- START -- ");
  // Serial to communicate with arduino
  Serial2.begin(57600);
  // One Wire Temp sensors
  Temperature.begin();
  // Setting up pins as inputs
  for (uint8_t i = 0; i < ANALOG_SEN; i++) pinMode(SEN[i], INPUT);
}

void loop()
{
  double data = 0, avg = 0;

  // Loop to record a large number of readings
  for (int reading = 0; reading < READINGS; reading++)
  {
    // Getting Current CT readings
    for ( int i = 0; i < 10; i++)
    {
      // Converting reading from digital to current
      data = ((analogRead(SEN[i]) - Offset[i]) / Slope[i]);
      if ( data < 0 ) data = 0;
      s[i].record(data);
      delay(READING_MS);
      // DEBUG
      //Serial.println("SEN: " + String(i) + " = " + String(data));
    }

    // Getting Voltage reading
    data = ((analogRead(SEN[10]) - Offset[10]) / Slope[10]);
    if ( data < 0 ) data = 0;
    s[10].record(data);
    delay(READING_MS);
    // DEBUG
    //Serial.println("SEN: 10 = " + String(data));

    // Getting Temperature reading
    Temperature.requestTemperatures();
    data = Temperature.getTempCByIndex(0);
    s[11].record(data);
    delay(READING_MS);
    // DEBUG
    //Serial.println("SEN: 11 = " + String(data));
  }

  Serial.println(" -- PRINT -- ");
  // Sending average readings to ESP8266
  for (int i = 0; i < TOTAL_SEN; i++)
  {
    avg = s[i].average();     // Getting the average
    Serial2.println("S" + String(i) + "_" + String(avg));
    delay(SERIAL_MS);
    // DEBUG
    Serial.println("S" + String(i) + "_" + String(avg));
  }

}




