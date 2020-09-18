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




const uint16_t kRecvPin = 14;
const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;

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

JsonObject& getConfig() {
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

  size_t size = f.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    //return "file too large";
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  f.readBytes(buf.get(), size);

  StaticJsonDocument<200> configJson;
  auto error = deserializeJson(configJson, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
  }
 
//  Serial.println("File Content:");
// 
//  while (f.available()) {
//    //configText += f.read();
//    Serial.write(f.read());
//  }
//  //Serial.println(configText);
//  return configText;
  Serial.println(configJson.as<String>());
  f.close();
  JsonObject configObj = configJson.as<JsonObject>();
  return configObj;
}

// This section of code runs only once at start-up.
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.setTimeout(30);
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  Serial.println("connected...yay! :)");

  getConfig();


  StaticJsonDocument<58> uuidJson;
  auto error = deserializeJson(uuidJson,getUuid());
  if (error) {
    Serial.println("Failed to parse config file");
    
  }
  Serial.println(uuidJson["uuid"].as<String>());

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
    //Serial.println(resultToSourceCode(&results));
    String wandid = uint64ToString(results.address, HEX);
    String wandintensity = uint64ToString(results.command, HEX);
    if (wandid != "0") {
      Serial.println("Wand: " + wandid);
      Serial.println("Intensity: " + wandintensity);
      http.begin("http://192.168.1.81:8080/function/graph2");  //Specify request destination
      http.addHeader("Content-Type", "text/plain");  //Specify content-type header
      int httpCode = http.POST("query { users { firstname lastname }}");                                                                  //Send the request

      if (httpCode > 0) { //Check the returning code

        String payload = http.getString();   //Get the request response payload
        Serial.println(payload);                     //Print the response payload

      }

      http.end();
    }
    Serial.println();    // Blank line between entries
    delay(500);
    yield();             // Feed the WDT (again)
  }
}
