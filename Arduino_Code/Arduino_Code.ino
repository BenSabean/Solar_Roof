#include <OneWire.h>
#include <DallasTemperature.h>

int Sensor[10] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9};

void setup()
{
  // Serial for Monitoring
  Serial.begin(9600);
  Serial.print("-START-");
  // Serial to communicate with arduino
  Serial2.begin(9600);
  // Setting up pins as inputs
  for (uint8_t i = 0; i < 10; i++) pinMode(Sensor[i], INPUT);
}

void loop()
{
  float temp;
  // Programming to move throught Current Array and calculating the current passing through each sensor
  for ( int i = 0; i < 10; i++)
  {
    Serial.print("Sensor " + String(i));
    temp = analogRead(Sensor[i]);     Serial.println("Raw AtoD = " + String(temp));
    temp = map(temp, 0, 1024, 0, 5);  Serial.println("Raw Voltage = " + String(temp));      // (Value[i] * 5) / 1024;
    temp = (temp / 240) - 0.012;      Serial.println("Raw Sensor Value = " + String(temp));
    temp = temp * 2500;               Serial.println("Final Output = " + String(temp));
    // Printing for ESP8266
    Serial2.println(String(temp));
  }

  delay(5000);
}




