#include <OneWire.h>
#include <DallasTemperature.h>

#define DELAY_S   5     // Delay for taking readings
#define ONE_WIRE  3     // Pin for connecting OneWire sensors

// One Wire setup
OneWire oneWire(ONE_WIRE);
DallasTemperature Temperature(&oneWire);

int Sensor[11] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10};

void setup()
{
  // Serial for Monitoring
  Serial.begin(9600);
  Serial.print("-START-");
  // Serial to communicate with arduino
  Serial2.begin(9600);
  // One Wire Temp sensors
  sensors.begin(); 
  // Setting up pins as inputs
  for (uint8_t i = 0; i < 11; i++) pinMode(Sensor[i], INPUT);
}

void loop()
{
  float data;
  // Programming to move throught Current Array and calculating the current passing through each sensor
  Serial.println("START");
  Serial2.println("START");
  // Getting Current CT readings
  for ( int i = 0; i < 10; i++)
  {
    Serial.print("Sensor " + String(i+1));
    data = analogRead(Sensor[i]);     Serial.println("Raw AtoD = " + String(data));
    data = map(data, 0, 1024, 0, 5);  Serial.println("Raw Voltage = " + String(data));      // (Value[i] * 5) / 1024;
    data = (data / 240) - 0.012;      Serial.println("Raw Sensor Value = " + String(data));
    data = data * 2500;               Serial.println("Final Output = " + String(data));
    // Printing for ESP8266
    Serial2.println(String(data));
  }
  // Getting Voltage Transducer reading
  data = analogRead(Sensor[10]);      Serial.println("Vol Raw AtoD = " + String(data));
  data = map(data, 0, 1024, 0, 5);    Serial.println("Vol Raw Voltage = " + String(data));
  Serial2.println(String(data));

  // Getting Temperature
  sensors.requestTemperatures();
  data = sensors.getTempCByIndex(0);  Serial.println("Temp = " + String(data));
  Serial2.println(String(data));

  // Sleep
  delay(DELAY_S * 1000);
}




