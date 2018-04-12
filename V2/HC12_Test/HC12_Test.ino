/*
   Arduino Long Range Wireless Communication using HC-12
                      Example 01
   by Dejan Nedelkovski, www.HowToMechatronics.com
*/
#include <SoftwareSerial.h>

SoftwareSerial HC12(2, 3); // HC-12 TX Pin, HC-12 RX Pin


void setup() 
{
  Serial.begin(115200);             // Serial port to computer
  HC12.begin(9600);               // Serial port to HC12

  Serial.println("START");
}


void loop() 
{
  delay(1000);
  HC12.print('*');
}
