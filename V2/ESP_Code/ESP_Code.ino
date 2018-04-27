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

#define SEN_NUM             14      // Number of sensor expected from Arduino
#define LOOP_DELAY_MS       100      // Delay for esp8266 loop

// Wifi setup //
#define DEVICE_ID           4
const char ssid[] =        "SolarRoof";
const char password[] =    "7F61C6A749";
AERClient server(DEVICE_ID);

// SoftwareSerial //
#define RX  2
#define TX  16
SoftwareSerial Arduino(RX, TX); // Antenna 12 14 Vcc
bool sen_flag[SEN_NUM];
char data[SEN_NUM][20];

// SD Card //
#define CS 15
volatile bool SDin = false;          // SD card indicator
volatile bool RTCin = false;         // RTC indicator

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
  if (!SD.begin(CS))  Serial.println("Cannot find SDCard");
  else                SDin = true;
  // Communication
  Arduino.begin(9600); // 38400
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
  // Clearing sensor flags
  for (int i = 0; i < SEN_NUM; i++) sen_flag[i] = false;
}

void loop()
{
  int index = 5;
  bool header_printed, published, store = true;
  char buff[100], *value;
  memset(buff, NULL, sizeof(buff));
  DateTime dateTime;
  File dataFile;

  /* Store received sensors from Arduino */
  if (Arduino.available() > 0)
  {
    // Reading incomming data
    readString(buff, sizeof(buff));
    // Error checking and saving
    if (buff[0] == 'S')
    {
      // Extracting index from S<10> 5.4223
      index = String( strtok(buff, " ") + 1 ).toInt();
      // Extracting data from S10 <5.4223>
      value = strtok(NULL, " ");
      // Error checking and saving
      if (index < SEN_NUM && index >= 0)
      {
        strcpy(data[index], value);
        // Increment sensor count
        sen_flag[index] = true;
      }
    }
  }

  // Looping to check if we received all the sensors
  // If one sensor is false - store is false and recording loop isnot entered
  for (int i = 0; i < SEN_NUM; i++)
    if (sen_flag[i] == false) store = false;

  // Every SEN_NUM reading publish and record data to SD card
  if (store)
  {
    for (int i = 0; i < SEN_NUM; i++)
      sen_flag[i] = false;  // Reset message counter after receiving 12 sensor

    // Publishing and printing the array
    for (int s = 0; s < SEN_NUM; s++)
    {
      // Converting int to char and publishing to IoT server
      sprintf(buff, "Data/Sensor%d", s + 1);
      bool posted = server.publish(buff, data[s]);
      //
      sprintf(buff, "Sensor%d %s", s, data[s]);
      Serial.println(buff); // DEBUG
      //
    }
    Serial.println();                 // DEBUG

    // Writing files to SD card
    if (SDin && RTCin)
    {
      dateTime = rtc.now();      // Getting Time
      //
      sprintf(buff, "%02d:%02d:%02d", dateTime.hour(), dateTime.minute(), dateTime.second());
      Serial.println(buff);
      //
      // Creating File Name for storage
      sprintf(buff, "%02d_%02d_%02d.csv", dateTime.day(), dateTime.month(), dateTime.year() - 2000);
      //
      Serial.println("File: " + String(buff));
      //
      header_printed = SD.exists(buff);       // Check if file already exist
      dataFile = SD.open(buff, FILE_WRITE);   // Opening the file
      if (dataFile)
      {
        // Header
        if (!header_printed) printHeaders(&dataFile);    // printing headers

        // Printing time - Format -> YYYY/MM/DD hh:mm:ss
        // dateTime
        sprintf(buff, "%04d/%02d/%02d %02d:%02d",
                dateTime.year(), dateTime.month(), dateTime.day(), dateTime.hour(), dateTime.minute());
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

      // Reseting data strings
      for (int i = 0; i < SEN_NUM; i++) memset(data[i], '\0', sizeof(data[i]));
    }

  }
  delay(LOOP_DELAY_MS);
}

/*
   Printing Header into to the SD Card
*/
void printHeaders (File *f)
{
  char header[10];
  uint8_t number = 0;

  f->print("Time stamp");
  for (int i = 1; i <= SEN_NUM; i++) // Printing sensor addresses
  {
    if (i <= 10)    sprintf(header, ",Current %d", i);
    else if (i <= 12)sprintf(header, ",Voltage %d", i - 10);
    else            sprintf(header, ",Temperature %d", i - 12);
    f->print(header);
  }
  f->println();     //create a new row to read data more clearly
}

/*
   Sets time on the RTC - Only needed when time drifts!
*/
void RTC_setTime()
{
  // following line sets the RTC to the date & time this sketch was compiled
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // Manutally Adjust time Year, Month, Day, Hour, Minute, Second
  rtc.adjust(DateTime(2018, 4, 5, 10, 23, 30));
}

/*
   Read string of characters from serial monitor
*/
void readString (char* buff, int len)
{
  int i;

  // Delay to wait for data to come in
  delay(10);
  for (i = 0; i < len && Arduino.available() > 0; i++)
    buff[i] = Arduino.read();
  buff[i - 2] = '\0';

  /*
    String msg;
    msg = Arduino.readString();
    strcpy(buff, msg.c_str());
  */
}

