#include <Arduino.h>
#include "DHT.h"

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>

#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()

#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include "Wire.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

// Display Settings
const int I2C_DISPLAY_ADDRESS = 0x3c;
const int SDA_PIN = D3;
const int SDC_PIN = D4;

StaticJsonDocument<256> configObj;
 
// replace with your channel's thingspeak API key, 
String apiKey = "KS3XLX9RADU7FBEB";
 
const char* server = "api.thingspeak.com";
#define DHTPIN 12 // what pin we’re connected to
 
DHT dht(DHTPIN, DHT22, 15);
float t = 0.;
float h = 0.;
WiFiClient client;


/*********************
 * End Settings
 *********************/

 // Initialize the oled display for address 0x3c
 // sda-pin=14 and sdc-pin=12
 SSD1306Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
 OLEDDisplayUi   ui( &display );

// Put all the config code here...
String getUuid() {
  HTTPClient http;
  // json size 16 + 42 = 58
  String uuid;
  http.begin("http://192.168.1.81:8080/function/getuuid");  //Specify request destination
  http.addHeader("Content-Type", "text/plain");  //Specify content-type header
  int httpCode = http.POST("query { users { firstname lastname }}"); //Send the request
  if (httpCode > 0) { //Check the returning code
    String payload = http.getString();   //Get the request response payload
    uuid = payload;                    //Print the response payload
  }
  http.end();
  return uuid;
}

String registerDevice(String confString) {
  String regString;
  HTTPClient http;
  http.begin("http://192.168.1.81:8080/function/graph2");  //Specify request destination
  http.addHeader("Content-Type", "text/plain");  //Specify content-type header
  int httpCode = http.POST("mutation { createDevice(device: "+confString+"){ uuid hardware sensor actuator }}"); //Send the request
  if (httpCode > 0) { //Check the returning code
    String payload = http.getString();   //Get the request response payload
    regString = payload;                    //Print the response payload
  }
  http.end();
  return regString;
}
String getConfig() {
  String configText;
  // json size ~256
  if(SPIFFS.begin())
  {
    Serial.println("SPIFFS Initialize....ok");
  }
  else
  {
    Serial.println("SPIFFS Initialization...failed");
  }
  File f = SPIFFS.open("/config.json", "r");
  if (!f) {
    Serial.println("Failed to open file for reading");
    //return "Error opening file.";
  }
  while (f.available()) {
    configText += char(f.read());
  }
  //Serial.println(configText);
  return configText;
  f.close();
}

void setConfig(String confString) {
  File file = SPIFFS.open("/config.json", "w");
 
  if (!file) {
    Serial.println("Error opening file for writing");
    return;
  }
 
  int bytesWritten = file.print(confString);
 
  if (bytesWritten > 0) {
    Serial.println("File was written");
    Serial.println(bytesWritten);
 
  } else {
    Serial.println("File write failed");
  }
 
  file.close();
}

bool checkIfKnown(String myUuid){
  Serial.println("starting checkIfKnown");
  
  bool isKnown = false;
  String knownObjKnown;
  StaticJsonDocument<256> knownObj;
  HTTPClient http;

  while(isKnown == false) {
    Serial.println("checking...");
    http.begin("http://192.168.1.81:8080/function/graph2");  //Specify request destination
    http.addHeader("Content-Type", "text/plain");  //Specify content-type header
    int httpCode = http.POST("query { devices(where: { uuid: \""+myUuid+"\"}){ known }}"); //Send the request
    if (httpCode > 0) { //Check the returning code
      String payload = http.getString();   //Get the request response payload
      Serial.println("payload: "+payload);
      auto error2 = deserializeJson(knownObj,payload);
      if (error2) {
        Serial.println("parsing error");
      }
    }
    http.end();
    isKnown = knownObj["devices"][0]["known"];
    if (!isKnown){ delay(3000); }
  }
  
  Serial.println("known. moving on.");
}
//End of config code....
   
 
void setup() {   
  Serial.begin(115200);
  delay(10);
  dht.begin();

  // initialize dispaly
  display.init();
  display.clear();
  display.display();

  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);
               
  pinMode(LED_BUILTIN, OUTPUT);

  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFiManager wifiManager;
  
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 10, "Connecting to WiFi");
    display.display();

  //wifiManager.resetSettings();
  wifiManager.setTimeout(30);
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  Serial.println("connected...huzzah! :)");
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 10, "Connected!");
    display.display();
  digitalWrite(LED_BUILTIN, HIGH);

  configObj;
  auto error2 = deserializeJson(configObj,getConfig());
  if (error2) {
    Serial.println("Failed to parse config file");
    
  }
  Serial.println(configObj.as<String>());

  if (configObj["device"]["uuid"] == ""){
    StaticJsonDocument<58> uuidJson;
    auto error = deserializeJson(uuidJson,getUuid());
    if (error) {
      Serial.println("Failed to parse uuid file");
      
    }
    Serial.println(uuidJson["uuid"].as<String>());
    configObj["device"]["uuid"] = uuidJson["uuid"].as<String>();
    setConfig(configObj.as<String>());
  }
  Serial.println(configObj.as<String>());

  // Registration code
   String regResult;
   //String regObj = configObj["device"].as<String>();
   String regObj = "";
   regObj += "{uuid:\""+configObj["device"]["uuid"].as<String>()+"\",";
   regObj += "hardware:\""+configObj["device"]["hardware"].as<String>()+"\",";
   regObj += "sensor:"+configObj["device"]["sensor"].as<String>()+",";
   regObj += "actuator:"+configObj["device"]["actuator"].as<String>()+"}";
   Serial.println("reg string: "+regObj);
   regResult = registerDevice(regObj);
   Serial.println(regResult);

   checkIfKnown(configObj["device"]["uuid"].as<String>());

   
  
}
 
 
void loop() {

  display.clear();
  display.display();
  
  h = dht.readHumidity();
  t = dht.readTemperature(true);
  Serial.println(t);
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    return;
  }
 
  if (client.connect(server,80)) {  //   "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(t);
           postStr +="&field2=";
           postStr += String(h);
           postStr += "\r\n\r\n";
 
     client.print("POST /update HTTP/1.1\n"); 
     client.print("Host: api.thingspeak.com\n"); 
     client.print("Connection: close\n"); 
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n"); 
     client.print("Content-Type: application/x-www-form-urlencoded\n"); 
     client.print("Content-Length: "); 
     client.print(postStr.length()); 
     client.print("\n\n"); 
     client.print(postStr);

     Serial.print("Temperature: ");
     Serial.print(t);
     Serial.print(" degrees Farenheit Humidity: "); 
     Serial.print(h);
     Serial.println("% send to Thingspeak");  
  }
  client.stop();

  Serial.println("ready to display");
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 0, "Indoor");
  display.drawString(64, 10, String(t, 1) + "°F");
  display.drawString(64, 30, String(h, 1) + "%");
  display.display();

  Serial.println("Waiting...");    
  // thingspeak needs minimum 15 sec delay between updates
  delay(20000);  
}
