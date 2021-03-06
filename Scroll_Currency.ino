/*
  Scroll_Currency use for watching the currency value on 8x8 LED Scrolling.
  Check the API : data.fixer.io ( need to have a account, free for 1000 request bhy month )
  It's necessary to have the acces_key 
  Http Request ( No https is needed )
  The software check eacg 30 minutes and update texte variable
  
  See Readme.md for specials Carcaters

  Hardware setup (ESP8266 <-> FC16):
  3V3 <-> VCC, GND <-> GND, D7 <-> DIN, D8 <-> CS, D5 <-> CLK


*/
// ===============================================================================================================================================
//                                                                   INCLUDE
// ===============================================================================================================================================
#include <ESP8266WiFi.h>
#include <FC16.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <NTPClient.h>                                                // Library for NTP Time (https://github.com/arduino-libraries/NTPClient)
#include <WiFiUdp.h>                                                  // Need for NTP
// ===============================================================================================================================================
//                                                                   DEFINE
// ===============================================================================================================================================
#define Version  "1.0c"
// ===============================================================================================================================================
//                                                                   VARIABLE
// ===============================================================================================================================================
unsigned long previousMillis = 1600000 ;
unsigned long interval = 1800000L;                                     
const char *host ="data.fixer.io";
String url = "/api/latest?access_key=xx";  
String payload;
const char* ssid     = "Freebox-60D420";
const char* password = "xx";
char Texte[100] = " \x11     \x01     \x03 ";
const int csPin = D8;			// CS pin used to connect FC16
const int displayCount = 8;		// Number of displays; usually 4 or 8
const int scrollDelay = 35;		// Scrolling speed - pause in ms
WiFiUDP ntpUDP;                                                       // Need for NTPClient
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 60000);     // NTP Object set the Offset (summer:7200/winter:3600)







FC16 display = FC16(csPin, displayCount);
//DynamicJsonBuffer jsonBuffer;
WiFiClient client;
HTTPClient http;

// ===============================================================================================================================================
//                                                                   wifiConnect
// ===============================================================================================================================================
void wifiConnect()
{
  Serial.print("Connecting to AP");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
   }
  Serial.println("");
  Serial.println("WiFi connected");  
}
// ===============================================================================================================================================
//                                                                   RequestHTTP
// ===============================================================================================================================================
void RequestHTTP(){
  Serial.print("[HTTP] begin...\n");
  if (http.begin(client, "http://data.fixer.io/api/latest?access_key=519b83318765b42673900d80c6420a16")) {  
      Serial.print("[HTTP] GET...\n");
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
           payload = http.getString();
           Serial.println(payload);
           DynamicJsonDocument doc(4096);
           deserializeJson(doc, payload);
           JsonObject obj = doc.as<JsonObject>();
           long timestamp = obj[String("timestamp")];
           String date= obj[String("date")];
           String rates= obj[String("rates")];
           float CAD = obj[String("rates")][String("CAD")];
           float USD = obj[String("rates")][String("USD")];
           float CNY = obj[String("rates")][String("CNY")];
           Serial.print("Timestamp:");Serial.println(timestamp);
           Serial.print("rates:");Serial.println(rates);
           Serial.print("CAD:");Serial.println(CAD);
           Serial.print("USD:");Serial.println(USD);
           Serial.print("CNY:");Serial.println(CNY);
           sprintf(Texte,"\x10   %02i/%02i/%04i   %02i:%02i  - Dollar US : %.3f - Dollar CAD : %.3f - Yuan CNY : %.3f  \x11     \x01     \x03    \x01",day(timestamp),month(timestamp),year(timestamp),hour(timestamp)+2, minute(timestamp),USD,CAD,CNY);
           Serial.print("text:");Serial.println(Texte);
           display.setText(Texte);           
          }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
   }
 
}
// ===============================================================================================================================================
//                                                                   SETUP
// ===============================================================================================================================================
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("-------------- START --------------------");
  Serial.println(__FILE__ " " __DATE__ " " __TIME__);
  Serial.print(" Version "); Serial.print(Version); Serial.println(" - Date : 2020-05-03 by SHM  ");  
  wifiConnect();
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
	display.shutdown(false);	// turn on display
	display.setIntensity(2);	// set medium brightness
	display.clearDisplay();		// turn all LED off
  display.setText(Texte);
  timeClient.begin();                                                // Start NTP
}
// ===============================================================================================================================================
//                                                                   LOOP
// ===============================================================================================================================================
void loop() {

  if ( millis() - previousMillis >= interval) {    
    if ( timeClient.update() == true) {  
          Serial.println(timeClient.getFormattedTime());
          if (timeClient.getHours()<21 && timeClient.getHours()>6) { RequestHTTP(); } // request if heour <21 anf hour >6
    }
    previousMillis = millis(); 
  }
	display.update();
	delay(scrollDelay);
}
