//D0 <---> LED_online<-->3V

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"         //https://github.com/tzapu/WiFiManager
#include "DHT.h"

#define DHTPIN D5
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
std::unique_ptr<ESP8266WebServer> server;

const int interruptPin = 0; //GPIO 0 (Flash Button) 
const int LED=2;   //On board blue LED 
const int LED_online=16;   //D0 led out
long time_start,time_stop;
int sta=1;
float temp,humid,hic,temp_x,humid_x,hic_x;

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
  dht.begin();
  
  pinMode(LED,OUTPUT); 
  pinMode(LED_online,OUTPUT);  //led out
  pinMode(interruptPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING); 
  digitalWrite(LED,HIGH); //LED off 
  digitalWrite(LED_online,HIGH); //LED off 
  
  
}

void loop() {
  if(sta==1){
    Serial.println("loop");
    //--------------------------------
    float humid_x = dht.readHumidity();
    float temp_x = dht.readTemperature();
    float hic_x = dht.computeHeatIndex(temp, humid, false);
    if (isnan(humid_x) || isnan(temp_x)){
      //Serial.println("Failed to read from DHT sensor!");
      //return;
    }
    else{
      humid = humid_x;
      temp = temp_x;
      hic = hic_x;
    }
    
    Serial.print("Humidity: ");  Serial.print(humid);  Serial.print(" %\t");
    Serial.print("Temperature: ");  Serial.print(temp);  Serial.print(" *C ");
    Serial.print("Heat index: ");  Serial.print(hic);  Serial.print(" *C ");
    

    //--------------------------------
  }
  else{
    Serial.println("save mode");
    WiFiManager wifiManager;  
    //wifiManager.resetSettings();
    wifiManager.autoConnect("HumidTempA01");

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    sta = 1;
    digitalWrite(LED,HIGH); //LED off

    }
  //check connection  
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("connecte");
    digitalWrite(LED_online,LOW); //LED on
    
  }
  else{
    Serial.println("NOT connected");
    digitalWrite(LED_online,HIGH); //LED off
  }

  delay(500);
    
}

void handleInterrupt() { 
    Serial.println("Interrupt Detected");
    
    if(sta==0){   //out from save mode
      sta = 1;
      digitalWrite(LED,HIGH); //LED off
      Serial.println("OUT from save mode");
      ESP.reset();
      
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
