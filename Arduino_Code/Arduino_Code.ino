#include <OneWire.h>
#include <DallasTemperature.h>

#define SEN_NUM       12          // Number of sensor connected 10 Current 1 Voltage 1 Temp

#define DELAY_S       5           // Delay for taking readings
#define ONE_WIRE      3           // Pin for connecting OneWire sensors
#define CURRENT_COEFF 0.04405286  // Multiplier to conver Digital to Current
#define VOLT_COEFF    0.5859375   // Multiplier to conver Digital to Voltage
#define MIDDLE_RANGE  576         // Value for 0 current flow
#define READINGS      100           // Average every reading value

// One Wire setup
OneWire oneWire(ONE_WIRE);
DallasTemperature Temperature(&oneWire);

int Sensor[11] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10};

float Slope[11] = {
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
  0     // S10
};

float Offset[11] = {
  577,    // S0 576.41
  578,    // S1 577.47
  571,    // S2 570.38
  576,    // S3  576.3
  576,    // S4  575.6
  575,    // S5 573.24
  579,    // S6 576.83
  575,    // S7 574.63
  573,    // S8  571.2
  575,    // S9 574.83
  0     // S10
};


float Buffer[SEN_NUM][READINGS];

int Reading = -1;

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
  float data, avg = 0;
  int s = 0, r = 0;
  // Programming to move throught Current Array and calculating the current passing through each sensor
  Reading ++;
  //Serial.println(" -- Reading " + String(Reading) + " --");

  // Getting Current CT readings
  for ( int i = 0; i < 10; i++)
  {
    //Serial.print("Sensor " + String(i));
    data = analogRead(Sensor[i]);       //Serial.print(" Raw AtoD = " + String(data));
    //Serial.print("\tRaw Voltage = " + String(data * 5 / 1024));
    data -= Offset[i];
    data /= Slope[i];              //Serial.print("\tRaw Current = " + String(data));
    //data *= (-1);
    if ( data < 0 ) data = 0;
    Buffer[i][Reading] = data;
    //Serial.print("\tFinal Current = " + String(data) + "\n");
    //Serial2.println("S" + String(i) + "_" + String(data));
    delay(10);
  }

  // Getting Voltage Transducer reading
  data = analogRead(Sensor[10]);        //Serial.print("Sensor 11 Raw AtoD = " + String(data));
  data *= VOLT_COEFF;                   //Serial.println("\tVol Raw Voltage = " + String(data / 1024 * 5));
  Buffer[10][Reading] = data;
  //Serial2.println("S10_" + String(data));
  delay(10);

  // Getting Temperature
  Temperature.requestTemperatures();
  data = Temperature.getTempCByIndex(0);  //Serial.println("Temp = " + String(data));
  Buffer[11][Reading] = data;
  //Serial2.println("S11_" + String(data));
  delay(10);


  if (Reading >= READINGS-1)
  {
    Serial.println("Printing loop");
    for (s = 0; s < SEN_NUM; s++)
    {
      avg = 0;
      for (r = 0; r < Reading; r++)
        avg += Buffer[s][r];
      Serial.println("Sensor " + String(s) + ": " + String(avg / Reading));
    }
    Reading = 0;
  }

  // Sleep
  //delay(1000);
}




