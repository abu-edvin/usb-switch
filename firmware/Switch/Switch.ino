#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "Proxyon";
const char* password = "typewriter";

ESP8266WebServer server(80);

void handleOn() {
  digitalWrite(LED_BUILTIN, LOW);
  Serial.print("1");
}

void handleOff() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("0");
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
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
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
  }
}
