#include <SoftwareSerial.h>         // For Arduino communication

#define SEN_NUM             12      // Number of sensor expected from Arduino


// SoftwareSerial //
#define RX  4
#define TX  12
SoftwareSerial Arduino(RX, TX);

void setup()
{
  // ----------
  Serial.begin(230400);
  Serial.println("START");
  // ----------

  // Communication
  Arduino.begin(57600);
  Arduino.readString();
  

}

void loop()
{
  char message[200];
  memset(message, NULL, sizeof(message));

  if(Arduino.available())
  {
    readString(message, sizeof(message));
    //Serial.print("GOT: ");
    Serial.println(message);
  }
  
  delay(200);
}




/*
   Read string of characters from serial monitor
*/
void readString (char* buff, int len)
{
  for(int i = 0; i < len && Arduino.available() > 0; i++)
    buff[i] = Arduino.read();
}

