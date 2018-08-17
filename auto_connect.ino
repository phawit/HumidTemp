#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"         //https://github.com/tzapu/WiFiManager

std::unique_ptr<ESP8266WebServer> server;

const int interruptPin = 0; //GPIO 0 (Flash Button) 
const int LED=2; //On board blue LED 
long time_start,time_stop;
int sta=1;

void handleRoot() {
  server->send(200, "text/plain", "hello from esp8266!");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += (server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";
  for (uint8_t i = 0; i < server->args(); i++) {
    message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
  }
  server->send(404, "text/plain", message);
}

void setup() {

  Serial.begin(115200);
  pinMode(LED,OUTPUT); 
  pinMode(interruptPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING); 
  digitalWrite(LED,HIGH); //LED off 
  
  
}

void loop() {
  if(sta==1){
    Serial.println("loop");
  }
  else{
    Serial.println("save mode");
    WiFiManager wifiManager;  
    wifiManager.resetSettings();
    wifiManager.autoConnect("HumidTempA01");

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    sta = 1;
//    server.reset(new ESP8266WebServer(WiFi.localIP(), 80));
//    server->on("/", handleRoot);
//    server->on("/inline", []() {
//      server->send(200, "text/plain", "this works as well");
//    });
//    server->onNotFound(handleNotFound);
//    server->begin();
//    Serial.println("HTTP server started.....");
//    Serial.println(WiFi.localIP());
    }
  delay(500);
    //server->handleClient();
}

void handleInterrupt() { 
    Serial.println("Interrupt Detected");
    
    if(sta==0){   //out from save mode
      sta = 1;
      digitalWrite(LED,HIGH); //LED off
      Serial.println("OUT from save mode");
      
    }
    else{   //to savemode
      time_start = millis();
      while(digitalRead(interruptPin)==0){
        if(millis()-time_start > 3000){
           Serial.println("save mode");
           sta = 0;
           digitalWrite(LED,LOW); //LED on 
           break;
        }
      }
    }
    
    
    
}
