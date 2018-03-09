/*
   Reads Serial data from Arduino Mega DAQ module
   And send it over MQTT to aerlab.ddns.net server
   dateTime: Feb1, 2018
*/
#include <ESP8266WiFi.h>            // ESP WiFi Libarary
#include <PubSubClient.h>           // MQTT publisher/subscriber client
#include <stdio.h>                  // for sprintf function
#include <SoftwareSerial.h>         // For Arduino communication
#include <SPI.h>                    // For SD card read/write
#include <SD.h>                     // For SD card read/write
#include <Wire.h>
#include "RTClib.h"                 // For RTC - DS1387 clock
#include <AERClient.h>              // Custom made Library for IoT server

#define SEN_NUM             12      // Number of sensor expected from Arduino

/* Wifi setup */
#define DEVICE_ID           4
const char ssid[] =        "AER172-2";
const char password[] =    "80mawiomi*";
AERClient server(DEVICE_ID);

// SoftwareSerial //
#define RX  12
#define TX  14
SoftwareSerial Arduino(RX, TX, false, 256);

// SD Card //
#define CS 15
File dataFile;
volatile int Message_count = 0;     // Counting sensors for SD writing

// RealTimeClock //
RTC_PCF8523 rtc;

void setup()
{
  // ----------
  Serial.begin(115200);
  Serial.println("START");
  // ----------
  // RTC initialization
  if (! rtc.begin())
    Serial.println("Couldn't find RTC");
  // Communication
  Arduino.begin(57600);
  // Start wifi and server communication
  server.init(ssid, password);
  Serial.println("\nCONNECTED");
  // Uncomment to debug WiFi and server connection
  //server.debug();
  // Uncomment to set RTC time if its drifted
  //RTC_setTime();

}

void loop()
{
  String FileName, Time, value;
  int index;
  bool header_printed, published;
  char message[150], data[SEN_NUM][10];
  memset(message, NULL, 150);

  DateTime dateTime = rtc.now();      // Getting Time
  /*
    Serial.println(                     // Printing
    String(dateTime.year()) + "/" +
    String(dateTime.month()) + "/" +
    String(dateTime.day()) + " " +
    String(dateTime.hour()) + ":" +
    String(dateTime.minute())
    );
  */

  /* Store received sensors from Arduino */
  if (Arduino.available())
  {
    readString(message, sizeof(message));

    // Error checking and saving
    if (String(message).substring(0, 1) == "S")
    {
      index = (String(strtok(message, "_") + 1)).toInt();
      value = String(strtok(NULL, "_"));
      Serial.println("Parsed -> index = " + String(index) + " value = " + value);
      if (index < SEN_NUM && index >= 0)
      {
        Serial.println("Inside data Copy");
        strcpy(&data[index][0], value.c_str());
      }
      Message_count++;                 // Increment sensor count
    }
  }

  // Every 12 reading record data to SD card
  if (Message_count >= SEN_NUM)
  {
    Message_count = 0;
    for (uint8_t i = 0; i < SEN_NUM; i++)
      Serial.println("[" + String(i) + "] = " + String(data[i]));
  }

  delay(100);
  /*
    // Save on SD Card
    if (!rtc.begin())         Serial.println("Couldn't find RTC");
    else
    {
    if (!rtc.initialized()) Serial.println("RTC is NOT running!");
    else
    {
      if (!SD.begin(CS))    Serial.println("SD Card failed!");
      else
      {
        dateTime = rtc.now();      // Getting Time

        Serial.println(            // Printing
          String(dateTime.year()) + "/" +
          String(dateTime.month()) + "/" +
          String(dateTime.day()) + " " +
          String(dateTime.hour()) + ":" +
          String(dateTime.year())
        );
        // Creating File Name for storage
        FileName = String(dateTime.day()) + "_" + String(dateTime.month()) + "_" + String(dateTime.year()) + ".csv";
        header_printed = SD.exists(FileName);       // Check if file already exist
        dataFile = SD.open(FileName, FILE_WRITE);   // Opening the file
        if (dataFile)
        {
          // Header
          if (!header_printed) printHeaders();    // printing headers

          // dateTime
          dataFile.print(String(dateTime.year()) + "/" + String(dateTime.month()) + "/" + String(dateTime.day()) + ",");

          // Time
          Time = String(dateTime.hour()) + ":";
          if (dateTime.minute() < 10)
            Time = Time + "0" + String(dateTime.minute());
          else
            Time = Time + String(dateTime.minute());
          dataFile.print(Time);                 // Printing Time Stamp
          // Values
          for (int i = 0; i < SEN_NUM; i++) // Printing sensor headers
            dataFile.print("," + data[i]);
          // End Row
          dataFile.println();                     //create a new row to read data more clearly
          dataFile.close();                       //close file
        }
      }
    }
    }
  */
}

/*
   Printing Header into to the SD Card
*/
void printHeaders ()
{
  String header;
  uint8_t number = 0;
  dataFile.print("Date,Time");
  for (int i = 1; i <= SEN_NUM; i++) // Printing sensor addresses
  {
    if (i <= 10)  header = "Current";
    else          header = "Voltage";
    if (i > 10)   number = i - 10;
    else          number = i;
    dataFile.print("," + header + String(number));
  }
  dataFile.println();                  //create a new row to read data more clearly
}

/*
   Sets time on the RTC - Only needed when time drifts!
*/
void RTC_setTime()
{
  if (! rtc.initialized())
  {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

/*
   Read string of characters from serial monitor
*/
void readString (char* buff, int len)
{
  for (int i = 0; (Arduino.available() > 0) && (i < len); i++)
    buff[i] = (char) Arduino.read();
}

