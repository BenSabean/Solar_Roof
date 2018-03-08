#include <OneWire.h>
#include <DallasTemperature.h>

#define DELAY_S       2           // Delay for taking readings
#define ONE_WIRE      3           // Pin for connecting OneWire sensors
#define CURRENT_COEFF 0.04405286  // Multiplier to conver Digital to Current
#define VOLT_COEFF    0.5859375   // Multiplier to conver Digital to Voltage

// One Wire setup
OneWire oneWire(ONE_WIRE);
DallasTemperature Temperature(&oneWire);

int Sensor[11] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10};
float Offset[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setup()
{
  // Serial for Monitoring
  Serial.begin(115200);
  Serial.print("-START-");
  // Serial to communicate with arduino
  Serial2.begin(9600);
  // One Wire Temp sensors
  Temperature.begin();
  // Setting up pins as inputs
  for (uint8_t i = 0; i < 11; i++) pinMode(Sensor[i], INPUT);
}

void loop()
{
  float data;
  // Programming to move throught Current Array and calculating the current passing through each sensor
  Serial.println(" ---- LOOP ---- ");
  // Getting Current CT readings
  for ( int i = 0; i < 10; i++)
  {
    Serial.print("Sensor " + String(i + 1));
    data = analogRead(Sensor[i]);       Serial.print(" Raw AtoD = " + String(data));
    Serial.print("\tRaw Voltage = " + String(data * 5 / 1024));
    data *= CURRENT_COEFF;              Serial.println("\tRaw Current = " + String(data));
    data += Offset[i];                  Serial.print("\tFinal Current = " + String(data) + "\n");
    // Printing for ESP8266
    Serial2.println("S" + String(i) + "_" + String(data));
  }
  // Getting Voltage Transducer reading
  data = analogRead(Sensor[10]);      Serial.print("Sensor 11 Raw AtoD = " + String(data));
  data *= VOLT_COEFF;            Serial.println("\tVol Raw Voltage = " + String(data / 1024 * 5));
  Serial2.println("S10_" + String(data));

  // Getting Temperature
  Temperature.requestTemperatures();
  data = Temperature.getTempCByIndex(0);  Serial.println("Temp = " + String(data));
  Serial2.println("S11_" + String(data));

  // Sleep
  delay(DELAY_S * 1000);
}




