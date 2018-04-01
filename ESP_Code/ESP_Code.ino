/*
   Reads Serial data from Arduino Mega DAQ module
   And send it over MQTT to aerlab.ddns.net server
   Date: March 14, 2018
*/
#include <ESP8266WiFi.h>            // ESP WiFi Libarary
#include <PubSubClient.h>           // MQTT publisher/subscriber client
#include <SoftwareSerial.h>         // For Arduino communication
#include <SPI.h>                    // For SD card read/write
#include <SD.h>                     // For SD card read/write
#include <Wire.h>                   // I2C needed for RTC
#include "RTClib.h"                 // For RTC - DS1387 clock
#include <AERClient.h>              // Custom made Library for IoT server

#define SEN_NUM             12      // Number of sensor expected from Arduino
#define LOOP_DELAY_MS       100      // Delay for esp8266 loop

// Wifi setup //
#define DEVICE_ID           4
const char ssid[] =        "AER172-2";
const char password[] =    "80mawiomi*";
AERClient server(DEVICE_ID);

// SoftwareSerial //
#define RX  12
#define TX  14
SoftwareSerial Arduino(RX, TX); // Antenna 12 14 Vcc

// SD Card //
#define CS 15
File dataFile;
volatile bool SDin = false;          // SD card indicator
volatile bool RTCin = false;         // RTC indicator
volatile int Message_count = -1;     // Counting sensors for SD writing

// RealTimeClock //
RTC_PCF8523 rtc;

void setup()
{
  // ----------
  Serial.begin(230400);
  Serial.println("START");
  // ----------
  // RTC initialization
  if (! rtc.begin())  Serial.println("Cannot find RTC");
  else                RTCin = true;
  // SD card initialization
  //if (!SD.begin(CS))  Serial.println("Cannot find SDCard");
  //else                SDin = true;
  // Communication
  Arduino.begin(57600);
  // Start wifi and server communication
  server.init(ssid, password);
  //
  Serial.println("\nCONNECTED");
  //
  // Uncomment to debug WiFi and server connection
  //server.debug();
  // Uncomment to set RTC time if its drifted
  //RTC_setTime();
  // Flush serial buffer
  if (Arduino.available()) Arduino.readString();
}

void loop()
{
  int index = 5;
  bool header_printed, published;
  char buff[100], data[SEN_NUM][10], *value;
  memset(buff, NULL, sizeof(buff));
  DateTime dateTime;

  /* Store received sensors from Arduino */
  if (Arduino.available() > 0)
  {
    // Reading incomming data
    readString(buff, sizeof(buff));
    // Error checking and saving
    if (buff[0] == 'S')
    {
      // Extracting index from S<10>_5.4223
      index = String( strtok(buff, ":") + 1 ).toInt();
      // Extracting data from S10_<5.4223>
      value = strtok(NULL, ":");
      // Error checking and saving
      if (index < SEN_NUM && index >= 0) strcpy(data[index], value);
      // Increment sensor count
      Message_count++;
    }
  }

  // Every 12 reading publish and record data to SD card
  if (Message_count >= SEN_NUM - 1)
  {
    Message_count = 0;  // Reset message counter after receiving 12 sensor

    // Publishing and printing the array
    for (int s = 0; s < SEN_NUM; s++)
    {
      // Converting int to char and publishing to IoT server
      sprintf(buff, "SEN%d", buff);
      server.publish(buff, data[s]);    // Might crash !!
      //
      sprintf(buff, "SEN%d: %s", s, data[s]);
      Serial.println(buff); // DEBUG
      //
    }
    Serial.println();                 // DEBUG

/*
    // Writing files to SD card
    if (SDin && RTCin)
    {
      dateTime = rtc.now();      // Getting Time
      //
      Serial.println(String(dateTime.hour()) + ":" + String(dateTime.minute()) + ":" + String(dateTime.second()));
      //
      // Creating File Name for storage
      sprintf(buff, "%02d_%02d_%04d.csv", dateTime.day(), dateTime.month(), dateTime.year());
      //
      Serial.println("File: " + String(buff));
      //
      header_printed = SD.exists(buff);       // Check if file already exist
      dataFile = SD.open(buff, FILE_WRITE);   // Opening the file
      if (dataFile)
      {
        // Header
        if (!header_printed) printHeaders();    // printing headers

        // Printing time - Format -> YYYY/MM/DD hh:mm:ss
        // dateTime
        sprintf(buff, "%04d/%02d/%02d %02d:%02d:%02d", dateTime.year(), dateTime.month(), dateTime.day(), dateTime.hour(), dateTime.minute());
        dataFile.print(buff);

        // Values
        for (int s = 0; s < SEN_NUM; s++) // Printing sensor headers
        {
          sprintf(buff, ",%s", data[s]);
          dataFile.print(buff);
        }

        // End Row
        dataFile.println();                     //create a new row
        dataFile.close();                       //close file
        //
        Serial.println("SAVED");
        //
      }
    }
    */
  
  }

  delay(LOOP_DELAY_MS);
}

/*
   Printing Header into to the SD Card
*/
void printHeaders ()
{
  char header[10];
  uint8_t number = 0;

  dataFile.print("Timestamp");
  for (int i = 1; i <= SEN_NUM; i++) // Printing sensor addresses
  {
    if (i <= 10)  sprintf(header, "Current %d", i);
    else          sprintf(header, "Voltage %d", i - 10);
    dataFile.print(header);
  }
  dataFile.println();     //create a new row to read data more clearly
}

/*
   Sets time on the RTC - Only needed when time drifts!
*/
void RTC_setTime()
{
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // Manutally Adjust time Year, Month, Day, Hour, Minute, Second
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}

/*
   Read string of characters from serial monitor
*/
void readString (char* buff, int len)
{
  for (int i = 0; i < len && Arduino.available() > 0; i++)
    buff[i] = Arduino.read();
}

