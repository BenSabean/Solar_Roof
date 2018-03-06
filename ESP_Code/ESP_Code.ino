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
#include <RTClib.h>                 // For RTC - DS1387 clock
#include <AERClient.h>              // Custom made Library for IoT server

#define SEN_NUM 12                  // Number of sensors

//------------------Wi-Fi-----------------------//
#define DEVICE_ID 4
/* Wifi setup */
const char* ssid =        "AER172-2";
const char* password =    "80mawiomi*";
AERClient server(DEVICE_ID);

//------------SoftwareSerial------------------//
#define RX  2
#define TX  3
SoftwareSerial Arduino(RX, TX);

//----------------SD Card--------------------//
#define CS 15
File dataFile;

//--------------RealTimeClock----------------//
RTC_PCF8523 rtc;

void setup()
{
  // ----------
  Serial.begin(9600);
  Serial.println();
  // ----------
  // Communication
  Arduino.begin(9600);
  // Start wifi and server communication
  server.begin(ssid, password);
  // DEBUG
  server.debug();
}

void loop()
{
  String message, FileName, sensorName, data[SEN_NUM], Time;
  bool header_printed, published;
  DateTime dateTime;

  /* Store received sensors from Arduino */
  message = get_message();
  if (message == "START\n")
  {
    for (uint8_t i = 0; i < SEN_NUM; i++)
      data[i] = get_message();
  }
  /* Publishing sensor over MQTT */
  for (uint8_t i = 0; i < SEN_NUM; i++)
  {
    if (i < 10)   server.publish("Current" + String(i), data[i]);
    if (i == 10)  server.publish("Voltage", data[i]);
    if (i == 11)  server.publish("Temperature", data[i]);
  }
  /* Save on SD Card */
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
        Serial.println(
          String(dateTime.year()) + "/" +
          String(dateTime.month()) + "/" +
          String(dateTime.day()) + " " +
          String(dateTime.hour()) + ":" +
          String(dateTime.year())
        );
        FileName = String(dateTime.day()) + "_" + String(dateTime.month()) + "_" + String(dateTime.year()) + ".csv";
        header_printed = SD.exists(FileName);// Check if file already exist
        dataFile = SD.open(FileName, FILE_WRITE);
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
   Reads mesage from Arduino
*/
String get_message ()
{
  while (!Arduino.available());
  return Arduino.readString();
}

