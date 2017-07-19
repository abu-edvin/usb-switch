#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// TODO:
// Implement some kind of IP broadcast.
// Add OLED?

// Delay between individual blinks in patterns.
#define BLINK_DELAY 100
// Default delay between power and data pin relays.
#define DATA_DELAY 10
// GPIO pin for power relay.
#define PIN_POWER 5
// GPIO pin for data relay.
#define PIN_DATA 4
// Random +/- offset max for power-data relay delays for simulating different cable insertion speed.
#define JITTER_DELAY 8
// Default number of seconds for simulate command, when no number is specified.
#define SIMULATE_SECONDS 5

static const char* ssid = "Proxyon";
static const char* password = "typewriter";
const struct {
  byte init;
  byte wlan;
  byte on;
  byte off;
} patterns = { B11110000, B10110010, B10110111, B11101101 };

ESP8266WebServer server(80);

void handleOn() {
  digitalWrite(PIN_POWER, HIGH);
  delay(DATA_DELAY + random(JITTER_DELAY));
  digitalWrite(PIN_DATA, HIGH);    
  writePattern(patterns.on);
  setLed(true);
  Serial.println("1");
}

void handleOff() {
  digitalWrite(PIN_DATA, LOW);
  delay(DATA_DELAY + random(JITTER_DELAY));
  digitalWrite(PIN_POWER, LOW);
  writePattern(patterns.off);
  setLed(false);
  Serial.println("0");
}

void handleSimulate() {
  uint16_t seconds = server.hasArg("seconds") ? (uint16_t)server.arg("seconds").toInt() : SIMULATE_SECONDS;
  uint16_t jitterDelay = server.hasArg("jitterDelay") ? (uint16_t)server.arg("jitterDelay").toInt() : JITTER_DELAY;
  Serial.println("simulating");
  handleOn();
  delay(seconds + random(1000));
  handleOff();
  Serial.println("s");
}

void setLed(bool lit) {
  digitalWrite(LED_BUILTIN, lit ? LOW : HIGH);
}

void writePattern(byte pattern) {
  for (byte i = 0; i < 8; i++) {
    byte bit = pattern & 1;
    setLed((bool)bit);   
    pattern >>= 1;
    delay(BLINK_DELAY);
  }
}

void handleNotFound(){  
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_POWER, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  writePattern(patterns.init);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Initializing WLAN");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    writePattern(patterns.wlan);
    Serial.print(".");
  }
  setLed(false);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  MDNS.begin("esp8266");

  server.on("/on", [](){
    handleOn();
    server.send(200, "text/plain", "on");
  });

  server.on("/off", [](){
    handleOff();
    server.send(200, "text/plain", "off");
  });

  server.on("/simulate", [](){
    server.send(200, "text/plain", "simulate");
    handleSimulate();
  });

  server.onNotFound(handleNotFound);

  server.begin();
}

void loop(void){
  server.handleClient();
  switch (Serial.read())
  {
  case '1':
    handleOn();
    break;
  case '0':
    handleOff();
    break;
  case 's':
    handleSimulate();
    break;  
  }
}
