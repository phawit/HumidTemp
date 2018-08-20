/*D4 <---> LED_online<-->3V
 *DHT22<-->D6
 * 
 * 
*/


#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"         //https://github.com/tzapu/WiFiManager
#include "DHT.h"
#include <time.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN D6
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
std::unique_ptr<ESP8266WebServer> server;
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int interruptPin = 0; //GPIO 0 (Flash Button) 
const int LED=2;   //On board blue LED 
const int LED_online=D4;    //16;   //D0 led out
long time_start,time_stop;
int sta=1;

float temp,humid,hic;
String flag,water;
int train,rest,b;

int timezone = 7 * 3600;                    //ตั้งค่า TimeZone ตามเวลาประเทศไทย
int dst = 0;   
time_t now = time(nullptr);
struct tm* newtime = localtime(&now);
String tmpNow = "";

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
  lcd.begin();
  
  pinMode(LED,OUTPUT); 
  pinMode(LED_online,OUTPUT);  //led out
  pinMode(interruptPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING); 
  digitalWrite(LED,HIGH); //LED off 
  digitalWrite(LED_online,HIGH); //LED off 

  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");     //ดึงเวลาจาก Server
    Serial.println("\nWaiting for time");
    while (!time(nullptr)) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println("");
  
}

void loop() {
  if(sta==1){
    Serial.println("loop");
    //--------------------------------
    calTemp();
    pubLCD();
    

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

void calTemp(){
  
  float humidX = dht.readHumidity();
  float tempX = dht.readTemperature();
  
  
  if (isnan(humidX) || isnan(tempX)) {
    Serial.println("Failed to read from DHT sensor!");    
    
  }
  else{
    humid = humidX;
    temp = tempX;
  }
  
  hic = dht.computeHeatIndex(temp, humid, false);

  if(hic < 27){       flag = "White";   water = "1/2"; train = 60; rest = 0; }
  else if(hic <= 32){ flag = "Green";   water = "1/2"; train = 50; rest = 10; }
  else if(hic <= 39){ flag = "Yellow";  water = "1";   train = 45; rest = 15; }
  else if(hic <= 51){ flag = "Red";     water = "1";   train = 30; rest = 30; }
  else {              flag = "Black";   water = "1";   train = 20; rest = 40; }


  Serial.print("Humidity: ");
  Serial.print(humid);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" *C ");  
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(" %\n");
  Serial.println(flag);
  Serial.println(water);
  Serial.println(train);
  Serial.println(rest);

}

void pubLCD(){
  lcd.clear();
  Serial.println(NowString() + " "+ NowString2()); lcd.setCursor(0, 0); lcd.print(NowString2() + "     " + NowString());
  Serial.print("Temp: ");    Serial.print(temp);   lcd.setCursor(0, 1); lcd.print("Temp  : "); lcd.print(temp);
  Serial.print("\tHumid: "); Serial.print(humid);  lcd.setCursor(0, 2); lcd.print("Humid : "); lcd.print(humid); 
  Serial.print("\tFlag: ");  Serial.print(flag);   lcd.setCursor(0, 3); lcd.print("Flag  : "); lcd.print(flag);
  
}
String NowString() {
  
  now = time(nullptr);
  tm* newtime = localtime(&now);
  tmpNow = "";
  tmpNow += String(newtime->tm_hour);
  tmpNow += ":";
  
  int t = int(newtime->tm_min);
  if(t < 10){
    tmpNow +="0" + String(newtime->tm_min);
  }
  else{
    tmpNow += String(newtime->tm_min);
  }
  
  //tmpNow += String(newtime->tm_min);
  
  return tmpNow;
  
}

String NowString2() {
  
  now = time(nullptr);
  tm* newtime = localtime(&now);
  tmpNow = "";
  tmpNow += String(newtime->tm_mday);
  tmpNow += "-";
  tmpNow += String(newtime->tm_mon + 1);
  tmpNow += "-";
  tmpNow += String(newtime->tm_year + 1900 + 543);   
  return tmpNow;
  
}
