#include <WiFi.h>
#include <WiFiClientSecure.h>
WiFiClientSecure client;

#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

//#include <string>
#include <HTTPClient.h>
#include "time.h"

#include <ArduinoJson.h>

// select which pin will trigger the configuration portal when set to LOW
#define TRIGGER_PIN 0

int timeout = 120; // seconds to run for


// Set the number of entries to show - default is 10 - note that spacing between entries can push some out of the viewable area
const int calEntryCount = 10;

// Get calendar
char calendarServer[] = "script.google.com";
char calendarServer2[] = "script.googleusercontent.com";

// Write the path for your google script to fetch calendar events
String calendarRequest = "https://script.google.com/macros/s/[고유코드]/exec";

// Right now the calendarentries are limited to time and title
struct calendarEntries
{
  String calTime;
  String calTitle;
};

/////////////////////////////////////////////////////////
//                      FONCTIONS                      //
/////////////////////////////////////////////////////////
bool displayCalendar();
String getURL(String url);
bool getRequest(char *urlServer, String urlRequest);
/////////////////////////////////////////////////////////

// setup
void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  Serial.println("\n Starting");
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  client.setInsecure();

}

// loop
void loop() {
  if ( digitalRead(TRIGGER_PIN) == LOW) {
    WiFiManager wm;    

    //reset settings - for testing
    //wm.resetSettings();
  
    // set configportal timeout
    wm.setConfigPortalTimeout(timeout);

    if (!wm.startConfigPortal("OnDemandAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }

    else {
    //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");

      displayCalendar();
    }
  }
  
}


// Buffer size, 128 bytes in this case
#define RESP_BUFFER_LENGTH 128
// Pointer to actual buffer
uint8_t * _buffer = new uint8_t[RESP_BUFFER_LENGTH];
// String to hold the final response
String _responseString = "";
String newURL = "";

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long period = 5000;

// Main display code - assumes that the display has been initialized
bool displayCalendar() {
  // Getting calendar from your published google script
  Serial.println(F("Getting calendar"));

  // if connection to calendar does not succeed
  if ( !(getRequest(calendarServer, calendarRequest)) ) {
    Serial.println("False1");
    return false;
  }
   
  previousMillis = millis();
  while(client.connected() || ((currentMillis - previousMillis) < period) )
  {  
    currentMillis = millis(); 
    Serial.println("1");
    Serial.println(currentMillis - previousMillis);
    while (client.available())
    {
      Serial.println("2");
      // Fill the buffer and make a note of the total read length 
      int actualLength = client.read(_buffer, RESP_BUFFER_LENGTH);
      // If it fails for whatever reason
      if(actualLength <= 0)
      {
          // Handle as you see fit
          Serial.println(".....");
          //return -1;
      }
      // Concatenate the buffer content to the final response string
      // I used an arduino String for convenience
      // but you can use strcat or whatever you see fit
      _responseString += String((char*)_buffer).substring(0, actualLength);
      Serial.println(_responseString);

      if (_responseString.indexOf("HREF=") > 0){
        newURL = getURL(_responseString);
        Serial.println("new url: " + newURL);     
      }
    }
  }

  delete[] _buffer;
  // Clear buffer memory
  client.stop();
  
  Serial.println("before request");
  
  if ( !(getRequest(calendarServer2, newURL)) ) {
    Serial.println("False2");
    return false;
  }
  
  Serial.println("after request");
  while(client.connected())
  { 
    Serial.println("3");  
    while (client.available())
    {
      Serial.println("4");
      String outputStr = client.readString();
      Serial.println(outputStr);
    }
  }
  Serial.println("end of while loop");
      
  client.stop();
  
  return true;
}

// Generic code for getting requests - doing it a bit pure,
// as most libraries I tried could not handle the redirects from google
bool getRequest(char *urlServer, String urlRequest) {
  client.stop(); // close connection before sending a new request

  if (client.connect(urlServer, 443)) { // if the connection succeeds
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET " + urlRequest); // " HTTP/1.1"
    Serial.println("GET " + urlRequest);
    client.println("User-Agent: ESP32 LOLIN32 Receiver/1.1");
    client.println();

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 10000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        Serial.println("Connection timeout");
        return false;
      }
    }
  } else {
    Serial.println(F("Calendar connection did not succeed"));
    return false;
  }
  return true;
}

/* Extract URL from http redirect - used especially for google as they always redirect to a new temporary URL */
String getURL(String url) {
  String tagStr = "Location: ";
  String endStr = "\n";

  int indexFrom = url.indexOf(tagStr) + tagStr.length();
  int indexTo   = url.indexOf(endStr, indexFrom);
  
  return url.substring(indexFrom, indexTo);
}
