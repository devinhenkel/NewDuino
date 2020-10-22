/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "greenacres";
const char* password = "23skid00";

uint8_t Pwm1 = D1; //Nodemcu PWM pin 
uint8_t Pwm2 = D2; //Nodemcu PWM pin

//Seven segment pins attachecd with nodemcu pins  
int a0 = 2;  //Gpio-15 of nodemcu esp8266  
int a1 = 13;  //Gpio-13 of nodemcu esp8266    
int a2 = 12;  //Gpio-12 of nodemcu esp8266   
int a3 = 14;  //Gpio-14 of nodemcu esp8266  

 char* dir = "stop";
 int vel = 1023;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  
  pinMode(a0, OUTPUT);     
  pinMode(a1, OUTPUT);     
  pinMode(a2, OUTPUT);
  pinMode(a3, OUTPUT);  
  
  digitalWrite(a0, LOW); //Stop first motor
  digitalWrite(a1, LOW);

  digitalWrite(a2, LOW); //Stop second motor
  digitalWrite(a3, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname("rollerbot");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /forward") >= 0) {
              digitalWrite(a0, HIGH); //Start first motor
              digitalWrite(a1, LOW);
            
              digitalWrite(a2, HIGH); //Start second motor
              digitalWrite(a3, LOW);
              Serial.println("forward");
              dir = "forward";
            } else if (header.indexOf("GET /reverse") >= 0) {
              digitalWrite(a0, LOW); //Start first motor
              digitalWrite(a1, HIGH);
            
              digitalWrite(a2, LOW); //Start second motor
              digitalWrite(a3, HIGH);
              Serial.println("reverse");
              dir = "reverse";
            } else if (header.indexOf("GET /left") >= 0) {
              digitalWrite(a0, HIGH); //Start first motor
              digitalWrite(a1, LOW);
            
              digitalWrite(a2, LOW); //Start second motor
              digitalWrite(a3, HIGH);
              Serial.println("left");
              dir = "left";
            } else if (header.indexOf("GET /right") >= 0) {
              digitalWrite(a0, LOW); //Start first motor
              digitalWrite(a1, HIGH);
            
              digitalWrite(a2, HIGH); //Start second motor
              digitalWrite(a3, LOW);
              Serial.println("right");
              dir = "right";
            } else if (header.indexOf("GET /stop") >= 0) {
              digitalWrite(a0, LOW); //Stop first motor
              digitalWrite(a1, LOW);
            
              digitalWrite(a2, LOW); //Stop second motor
              digitalWrite(a3, LOW);
              Serial.println("stop");
              dir = "stop";
            } else if (header.indexOf("GET /speed25") >= 0) {
              analogWrite(Pwm1, 312);
              analogWrite(Pwm2, 312);
              Serial.println("25% Speed");
              vel = 312;
            } else if (header.indexOf("GET /speed50") >= 0) {
              analogWrite(Pwm1, 512);
              analogWrite(Pwm2, 512);
              Serial.println("50% Speed");
              vel = 512;
            } else if (header.indexOf("GET /speed75") >= 0) {
              analogWrite(Pwm1, 767);
              analogWrite(Pwm2, 767);
              Serial.println("75% Speed");
              vel = 767;
            } else if (header.indexOf("GET /speed100") >= 0) {
              analogWrite(Pwm1, 1023);
              analogWrite(Pwm2, 1023);
              Serial.println("25% Speed");
              vel = 1023;
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Rollerbot</h1>");
            
            // Display current state, and ON/OFF buttons for forward 
            client.println("<p>Forward</p>");
            // If the dir is not forward, it displays the ON button       
            if (dir!="forward") {
              client.println("<p><a href=\"/forward\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/stop\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for reverse 
            client.println("<p>Reverse</p>");
            // If the dir is not reverse, it displays the ON button       
            if (dir != "reverse") {
              client.println("<p><a href=\"/reverse\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/stop\"><button class=\"button button2\">OFF</button></a></p>");
            }
               
            // Display current state, and ON/OFF buttons for left 
            client.println("<p>Left</p>");
            // If the dir is not left, it displays the ON button       
            if (dir != "left") {
              client.println("<p><a href=\"/left\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/stop\"><button class=\"button button2\">OFF</button></a></p>");
            }
               
            // Display current state, and ON/OFF buttons for right 
            client.println("<p>Right</p>");
            // If the dir is not right, it displays the ON button       
            if (dir != "right") {
              client.println("<p><a href=\"/right\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/stop\"><button class=\"button button2\">OFF</button></a></p>");
            }
            // Speed buttons       
            if (vel != 312) {
              client.println("<p><a href=\"/speed25\"><button class=\"button\">25%</button></a></p>");
            } else {
              client.println("<p><a href=\"/speed25\"><button class=\"button button2\">25%</button></a></p>");
            }      
            if (vel != 512) {
              client.println("<p><a href=\"/speed50\"><button class=\"button\">50%</button></a></p>");
            } else {
              client.println("<p><a href=\"/speed50\"><button class=\"button button2\">50%</button></a></p>");
            }      
            if (vel != 767) {
              client.println("<p><a href=\"/speed75\"><button class=\"button\">75%</button></a></p>");
            } else {
              client.println("<p><a href=\"/speed75\"><button class=\"button button2\">75%</button></a></p>");
            }      
            if (vel != 1023) {
              client.println("<p><a href=\"/speed100\"><button class=\"button\">100%</button></a></p>");
            } else {
              client.println("<p><a href=\"/speed100\"><button class=\"button button2\">100%</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
