#include <OneWire.h>
#include <DallasTemperature.h>

#define SEN_NUM 12                // Number of sensor connected 10 Current 1 Voltage 1 Temp

#define DELAY_S       5           // Delay for taking readings
#define ONE_WIRE      3           // Pin for connecting OneWire sensors
#define CURRENT_COEFF 0.04405286  // Multiplier to conver Digital to Current
#define VOLT_COEFF    0.5859375   // Multiplier to conver Digital to Voltage
#define MIDDLE_RANGE  576

// One Wire setup
OneWire oneWire(ONE_WIRE);
DallasTemperature Temperature(&oneWire);

int Sensor[11] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10};
float Offset[11] = {0, 0, 0.4, 0.2, 0.2, 0, 0, 0, 0, 0, 0};
float Data[SEN_NUM];

void setup()
{
  // Serial for Monitoring
  Serial.begin(115200);                                                                        
  Serial.print("-START-");
  // Serial to communicate with arduino
  Serial2.begin(57600);
  // One Wire Temp sensors
  Temperature.begin();
  // Setting up pins as inputs
  for (uint8_t i = 0; i < 11; i++) pinMode(Sensor[i], INPUT);
}

void loop()
{
  float data;
  char buff[150] = {'\0'};
  String temp;
  // Programming to move throught Current Array and calculating the current passing through each sensor
  Serial.println(" ---- LOOP ---- ");
  // Getting Current CT readings
  for ( int i = 0; i < 10; i++)
  {
    Serial.print("Sensor " + String(i));
    data = analogRead(Sensor[i]);       Serial.print(" Raw AtoD = " + String(data));
    Serial.print("\tRaw Voltage = " + String(data * 5 / 1024));
    data -= MIDDLE_RANGE;
    data *= CURRENT_COEFF;              Serial.print("\tRaw Current = " + String(data));
    data += Offset[i];                  Serial.print("\tFinal Current = " + String(data) + "\n");
    Data[i] = data;
  }
  // Getting Voltage Transducer reading
  data = analogRead(Sensor[10]);      Serial.print("Sensor 11 Raw AtoD = " + String(data));
  data *= VOLT_COEFF;                 Serial.println("\tVol Raw Voltage = " + String(data / 1024 * 5));
  Data[10] = data;

  // Getting Temperature
  Temperature.requestTemperatures();
  data = Temperature.getTempCByIndex(0);  Serial.println("Temp = " + String(data));
  Data[11] = data;

  // Printing for ESP8266
  for (int i = 0; i < 11; i++)
  {
    temp = ("S" + String(i) + "_" + String(Data[i]) + ";");
    strcat(buff, temp.c_str());
  }

  //Serial.println(buff);
  Serial2.println(buff);
  
  // Sleep
  delay(5000);
}




