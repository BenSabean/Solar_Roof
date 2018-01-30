#include <OneWire.h>

#include <DallasTemperature.h>

int Current[10] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9};
float Value[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
float Volts[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
float Sensor_Value[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
float PV_LineCurrents[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};

void setup() 
{
  // put your setup code here, to run once:
Serial.begin(9600);
Serial.print("begin");
pinMode(A0, INPUT);
}

void loop() 
{
  
            // Programming to move throught Current Array and calculating the current passing through each sensor 
  for( int i = 0; i < 10; i++)
  {
    Value[i] = analogRead(Current[i]);
    Volts[i] =  (Value[i]*5)/1024;
    Sensor_Value[i] = (Volts[i]/240)-0.012;
    PV_LineCurrents[i] =  Sensor_Value[i]*2500;
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(" ; ");
    Serial.println(PV_LineCurrents[i]);
  }

  delay(5000);

}           
           
 


