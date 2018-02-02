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

#define SEN_NUM 10                  // Number of sensors

//------------------MQTT-----------------------//
#define MQTT_PORT
#define DEVICE_ID 1
/* Wifi setup */
const char* ssid =        "AER172-2";
const char* password =    "80mawiomi*";
/* MQTT connection setup */
const char* mqtt_user =   "aerlab";
const char* mqtt_pass =   "server";
const char* mqtt_server = "aerlab.ddns.net";
/* Topic Setup */
const char* ID = "4";
char gTopic[64];                             // Stores the gTopic being sent

WiFiClient espClient;
PubSubClient client(mqtt_server, 1883, espClient);
String clientName;

//------------SoftwareSerial------------------//
#define RX  2
#define TX  3
SoftwareSerial Arduino(RX, TX);

//----------------SD Card--------------------//
#define CS 15
File dataFile;

//--------------RealTimeClock----------------//
RTC_PCF8523 rtc;

// ------------------SETUP--------------------//
void setup()
{
  // ----------
  Serial.begin(9600);
  Serial.println();
  // ----------
  // Communication
  Arduino.begin(9600);

  client.setServer(mqtt_server, 1883);
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(DEVICE_ID);

  // ----------
  Serial.println("Module Name: " + clientName);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // ----------

  // WIFI Settings //
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  // ----------
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // ----------

}

void loop()
{
  String message, FileName, sensorName, data[SEN_NUM], Time;
  bool header_printed, published;
  DateTime dateTime;

  if (WiFi.status() == WL_CONNECTED)
  {
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
      if (i < 10) sensorName = String("Current");
      else sensorName = String("Voltage");
      sprintf(gTopic, "%s/%s/%s%i", ID, "Data", sensorName.c_str(), i);
      published = client.publish(gTopic, data[i].c_str());
    }
    /* Save on SD Card */
    if (!rtc.begin()) Serial.println("Couldn't find RTC");
    else
    {
      if (!rtc.initialized()) Serial.println("RTC is NOT running!");
      else
      {
        if (!SD.begin(CS)) Serial.println("SD Card failed!");
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

    // check for MQTT Connection
    if (!client.connected())
      reconnect();
  }
  else
  {
    WiFi.begin(ssid, password);
    delay(10000);
  }
}

/*
   If connection failed
*/
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName.c_str(), mqtt_user, mqtt_pass))
      Serial.println("connected");
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 30 seconds before retrying
      delay(30000);
    }
  }
}

/*
   Internal Mac address to string conversion
*/
String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
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

