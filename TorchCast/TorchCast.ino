#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRtext.h>
#include <IRutils.h>

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>


StaticJsonDocument<256> configObj;

const uint16_t kRecvPin = 14;
const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;

int isOn = 0;

#if DECODE_AC
const uint8_t kTimeout = 50;
#else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t kTimeout = 15;
#endif  // DECODE_AC

const uint16_t kMinUnknownSize = 12;
// ==================== end of TUNEABLE PARAMETERS ====================

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results

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

// This section of code runs only once at start-up.
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFiManager wifiManager;
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

  // Here starts the IR Code
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.enableIRIn();  // Start the receiver
}

// The repeating section of the code
void loop() {
  HTTPClient http;
  
  // Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    delay(30);
    //Serial.println(resultToSourceCode(&results));
    String wandid = uint64ToString(results.address, HEX);
    String wandintensity = uint64ToString(results.command, HEX);
    if (wandid != "0") {
      Serial.println("Wand: " + wandid);
      Serial.println("Intensity: " + wandintensity);
      Serial.println("UUID: "+configObj["device"]["uuid"].as<String>());
      //http.begin("http://192.168.1.81:8080/function/graph2");  //Specify request destination
      //http.addHeader("Content-Type", "text/plain");  //Specify content-type header
      //int httpCode = http.POST("query { devices(where: { uuid: \""+configObj["device"]["uuid"].as<String>()+"\"}){ name location { name } known }}");   //Send the request
      String serverName = "http://192.168.1.27/LED=";
      Serial.println(isOn);
      
      if (isOn!=1) {
        serverName = serverName + "ON";
        isOn = 1;
      } else {
        serverName = serverName + "OFF";
        isOn = 0;
      }
      
      http.begin(serverName);  //Specify request destination
      //http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //Specify content-type header
      String httpRequestData = "";
      int httpCode = http.GET();   //Send the request

      if (httpCode > 0) { //Check the returning code

        String payload = http.getString();   //Get the request response payload
        Serial.println(payload);                     //Print the response payload

      }

      http.end();

      http.end();
    }
    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)
  }
}
