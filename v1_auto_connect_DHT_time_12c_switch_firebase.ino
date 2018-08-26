/*D4 <---> LED_online<-->3V
 *DHT22<-->D6  
 *SWITCH<-->D8 //GPIO 15 touch switch
 *SCL<-->D1
 *SDA<-->D2
 *LED_online<-->D4  //yellow
 *LED_LED_savemode<-->D7;   //blue
*/


#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"         //https://github.com/tzapu/WiFiManager
#include "DHT.h"
#include <time.h>
#include <LiquidCrystal_I2C.h>
#include <FirebaseArduino.h>

#define DHTPIN D6
#define DHTTYPE DHT22
#define FIREBASE_HOST "humidtemp-59706.firebaseio.com"
#define FIREBASE_AUTH "i0FITeXlp0SgwgubVa580b3Zy42KX0Lw1FXDV3rB"   //Database Secret

DHT dht(DHTPIN, DHTTYPE);
std::unique_ptr<ESP8266WebServer> server;
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int interruptPin = 15;// 0; //GPIO 0 (Flash Button) 
const int LED=2;   //On board blue LED 
const int LED_online=D4;    //16;   //D0 led out
const int LED_savemode=D7;
long time_start,time_stop;
int sta=1;
int pub=0;
float iHumid=0,iTemp=0;

float temp,humid,hic;
String flag,water;
int train,rest,b;
int timeMin = -1;

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
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
  pinMode(LED,OUTPUT); 
  pinMode(LED_online,OUTPUT);  //led out
  pinMode(LED_savemode,OUTPUT);  //led out
  pinMode(interruptPin, INPUT); 
  pinMode(14, INPUT);  //touch switch 
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING); 
  digitalWrite(LED,HIGH); //LED off 
  digitalWrite(LED_online,HIGH); //LED off 
  digitalWrite(LED_savemode,HIGH); //LED off 


  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");     //ดึงเวลาจาก Server
    Serial.println("\nWaiting for time");
    while (!time(nullptr)) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println("");
    lcd.noBacklight();
  
}

void loop() {
  if(sta==1){
    Serial.println("loop");
    //--------------------------------
    calTemp();
    pubLCD();
    Serial.print("timeLog: ");
    Serial.print(timeLog());
    

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
    digitalWrite(LED_savemode,HIGH); //LED off

    }
  //check connection  
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("connecte");
    digitalWrite(LED_online,LOW); //LED on
    pubFirebase("A03",0);     //unit_id: A01,A02,A03,.. , float r: random val to make temp for another unit
    pubFirebase("A01",0.8);
    pubFirebase("A02",0.3);
    pubFirebase("A04",-0.3);

    if(int(newtime->tm_min)-timeMin >= 1){        //time update log firebase 1 min
      timeMin = int(newtime->tm_min);
      logFirebase("A03",0);
      logFirebase("A01",0.8);
      logFirebase("A02",0.3);
      logFirebase("A04",-0.3);
    }

    
  }
  else{
    Serial.println("NOT connected");
    digitalWrite(LED_online,HIGH); //LED off
  }
  //--------------------------
   if(b==1){   
    delay(3000);
    lcd.noBacklight();
    b = 0;
  }
  //---------------------------
  delay(5000);   //time update current firebase 5sec
    
}

void handleInterrupt() { 
    Serial.println("Interrupt Detected");
    lcd.backlight();
    b = 1;
    
    if(sta==0){   //out from save mode
      sta = 1;
      digitalWrite(LED,HIGH); //LED off
      digitalWrite(LED_savemode,HIGH); //LED off
      Serial.println("OUT from save mode");
      ESP.reset();
      
    }
    else{   //to savemode
      time_start = millis();
      while(digitalRead(interruptPin)==1){
        if(millis()-time_start > 3000){
           Serial.println("save mode");
           sta = 0;
           digitalWrite(LED,LOW); //LED on 
           digitalWrite(LED_savemode,LOW); //LED on 
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
    humid = humidX + iHumid;
    temp = tempX + iTemp;
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

String timeLog() {
  
  now = time(nullptr);
  tm* newtime = localtime(&now);
  tmpNow = "";
  tmpNow += String(newtime->tm_year + 1900);
  if(int(newtime->tm_mon + 1) < 10){
    tmpNow +="0" + String(newtime->tm_mon + 1);
  }
  else{
    tmpNow += String(newtime->tm_mon + 1);
  }
  if(int(newtime->tm_mday + 1) < 10){
    tmpNow +="0" + String(newtime->tm_mday + 1);
  }
  else{
    tmpNow += String(newtime->tm_mday + 1);
  }
  
  tmpNow += "-";
  if(int(newtime->tm_hour) < 10){
    tmpNow +="0" + String(newtime->tm_hour);
  }
  else{
    tmpNow += String(newtime->tm_hour);
  }
  tmpNow += ":";
  if(int(newtime->tm_min) < 10){
    tmpNow +="0" + String(newtime->tm_min);
  }
  else{
    tmpNow += String(newtime->tm_min);
  }
  tmpNow += ":";
  if(int(newtime->tm_sec) < 10){
    tmpNow +="0" + String(newtime->tm_sec);
  }
  else{
    tmpNow += String(newtime->tm_sec);
  }
     
  return tmpNow;
  
}

void pubFirebase(String unit_id,float r){     //unit_id: A01,A02,A03,.. , float r: random val to make temp for another unit
  //check firebase update
  pub = Firebase.getInt("ID/"+unit_id+"/Current/Pub");
  Serial.print("get Pub:  ");
  Serial.println(pub);
  if(pub == 0){
    Serial.println("Firebase NOT UPDATE..");
  }
  else{
    Serial.println("Firebase UPDATE..");
    iHumid = Firebase.getInt("ID/"+unit_id+"/Current/iHumid");
    iTemp = Firebase.getInt("ID/"+unit_id+"/Current/iTemp");
    Firebase.setInt("ID/"+unit_id+"/Current/Pub",0);
    pub = 0;
    
  }

  String UnitName = Firebase.getString("ID/"+unit_id+"/Current/UnitName");
  float Latitude = Firebase.getFloat("ID/"+unit_id+"/Current/Latitude");
  float Longtitude = Firebase.getFloat("ID/"+unit_id+"/Current/Longtitude");
  
  //Publish Firebase
  Serial.println("Publish Firebase");
  Firebase.setString("ID/"+unit_id+"/Current/DateTime", timeLog());
  Firebase.setFloat("ID/"+unit_id+"/Current/Temperature", temp+r);
  Firebase.setFloat("ID/"+unit_id+"/Current/Humid", humid+1.3*r);
  Firebase.setFloat("ID/"+unit_id+"/Current/HID", hic+0.8*r);
  Firebase.setString("ID/"+unit_id+"/Current/Flag", flag);
  Firebase.setInt("ID/"+unit_id+"/Current/Train", train);
  Firebase.setInt("ID/"+unit_id+"/Current/Rest", rest);
  Firebase.setString("ID/"+unit_id+"/Current/Water", water);
  //Firebase.setInt("ID/"+unit_id+"/Current/Latitude", latitude);
  //Firebase.setString("ID/"+unit_id+"/Current/Longtitude", longtitude);    
 
  if (Firebase.failed()) {
      Serial.print("set /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
    
}

void logFirebase(String unit_id,float r){
      
     String timeStamp = timeLog();
     Firebase.setString("ID/"+unit_id+"/Log/"+timeStamp+"/DateTime", timeLog());
     Firebase.setFloat("ID/"+unit_id+"/Log/"+timeStamp+"/Temperature", temp+r);
     Firebase.setFloat("ID/"+unit_id+"/Log/"+timeStamp+"/Humid", humid+1.3*r);
     Firebase.setFloat("ID/"+unit_id+"/Log/"+timeStamp+"/HID", hic+0.8*r);
     Firebase.setString("ID/"+unit_id+"/Log/"+timeStamp+"/Flag", flag);
     Firebase.setInt("ID/"+unit_id+"/Log/"+timeStamp+"/Train", train);
     Firebase.setInt("ID/"+unit_id+"/Log/"+timeStamp+"/Rest", rest);
     Firebase.setString("ID/"+unit_id+"/Log/"+timeStamp+"/Water", water);
     //Firebase.setInt("ID/"+unit_id+"/Log/"+timeStamp+"/Latitude", latitude);
     //Firebase.setString("ID/"+unit_id+"/Log"+timeStamp+"/Longtitude", longtitude); 
     
}
