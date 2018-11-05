/*D4 <---> LED_online<-->3V
 *DHT22<-->D6  
 *SWITCH<-->D8 //GPIO 15 touch switch----------
 *SWITCH<-->D4 //GPIO 2 touch switch
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
#include <EEPROM.h>

#define DHTPIN D6
#define DHTTYPE DHT22
#define FIREBASE_HOST "humidtemp-59706.firebaseio.com"
#define FIREBASE_AUTH "i0FITeXlp0SgwgubVa580b3Zy42KX0Lw1FXDV3rB"   //Database Secret

DHT dht(DHTPIN, DHTTYPE);
std::unique_ptr<ESP8266WebServer> server;
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int interruptPin = 15;// 0; //GPIO 0 (Flash Button) 
const int interruptPin2 = 14; // D5 setting
const int interruptPinAdd = 13;
const int LED=2;   //On board blue LED 
const int LED_online=D4;    //16;   //D0 led out
const int LED_savemode=D3;
long time_start,time_start1,time_stop;
int sta=1;
int sta2=0;
int staSet=0;
int staAdd = 0;
float staTemp;
float staHumid;
volatile byte state = LOW;
int staContinous = 0;

int pub=0;
float iHumid=0,iTemp=0;

float temp,humid,hic;
String flag,water;
int train,rest,b;
unsigned long timeUpdateLog = 0;
unsigned long timeUpdateData = 0;
unsigned long timeBacklight = 0;

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
  
  EEPROM.begin(512);
  staTemp = EEPROM_read(0, 5).toFloat();
  staHumid = EEPROM_read(10, 5).toFloat();
  
  pinMode(LED,OUTPUT); 
  pinMode(LED_online,OUTPUT);  //led out
  pinMode(LED_savemode,OUTPUT);  //led out
  pinMode(interruptPin, INPUT); 
  pinMode(interruptPin2, INPUT); 
  pinMode(interruptPinAdd, INPUT); 
  
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING); //RISING
  attachInterrupt(digitalPinToInterrupt(interruptPin2), handleInterrupt2, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPinAdd), handleInterruptAdd, CHANGE); 
  
   
  digitalWrite(LED,HIGH); //LED off 
  digitalWrite(LED_online,LOW); //LED off 
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

void handleInterrupt() {
  
  

 if(sta2==1){

  if(staSet==0){
      staTemp = staTemp-0.01;
      Serial.print("Temp: ");    Serial.print(temp+staTemp);   
      lcd.setCursor(8, 1); lcd.print(temp+staTemp); lcd.setCursor(14, 1); lcd.print("t"); lcd.print(staTemp);
  
    }
    if(staSet==1){
      staHumid = staHumid-0.01;
      Serial.print("\tHumid: "); Serial.print(humid+staHumid);  //lcd.setCursor(0, 2); lcd.print("Humid : "); lcd.print(humid+staHumid); 
      lcd.setCursor(8, 2); lcd.print(humid+staHumid); lcd.setCursor(14, 2); lcd.print("h"); lcd.print(staHumid);
    }
    
      
 }
 else{
    
    Serial.println("Interrupt Detected");
    lcd.backlight();
    b = 1;
    
    if(sta==0){   //out from save mode
      sta = 1;
      digitalWrite(LED,HIGH); //LED off
      digitalWrite(LED_savemode,HIGH); //LED off
      Serial.println("OUT from save mode");
      lcd.clear();
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

           lcd.backlight();
           lcd.clear(); lcd.setCursor(0,1); lcd.print("    Wifi Hospot!    ");
           lcd.setCursor(0,2); lcd.print("*******HT-A03*******");
    
           break;
        }
      }
    } 

 }
}



void loop() {
  
  while(sta2==1){
   
    while(staSet==0){
          
      while(digitalRead(interruptPinAdd)==1){
        if(millis()-time_start1 > 1000){
           Serial.println("contious mode");
           lcd.setCursor(8, 1); lcd.print(temp+staTemp); lcd.setCursor(14, 1); lcd.print("t"); lcd.print(staTemp);
           staTemp += 0.01;
           
           
           delay(50);
           
        }
        
      }

      while(digitalRead(interruptPin)==1){
        if(millis()-time_start1 > 1000){
           Serial.println("contious mode");
           lcd.setCursor(8, 1); lcd.print(temp+staTemp); lcd.setCursor(14, 1); lcd.print("t"); lcd.print(staTemp);
           staTemp -= 0.01;
           
           
           delay(50);
           
        }
        
      }
     
      Serial.print("staTemp::");  Serial.println(staTemp);
      Serial.println(digitalRead(interruptPinAdd));
      
      lcd.setCursor(8, 1); lcd.print("     "); lcd.setCursor(14, 1); lcd.print("      ");
      delay(500);
      lcd.setCursor(8, 1); lcd.print(temp+staTemp); lcd.setCursor(14, 1); lcd.print("t"); lcd.print(staTemp);
      delay(500);
    
    }
    
    while(staSet==1){
      while(digitalRead(interruptPinAdd)==1){
        if(millis()-time_start1 > 1000){
           Serial.println("contious mode");
           lcd.setCursor(8, 2); lcd.print(humid+staHumid); lcd.setCursor(14, 2); lcd.print("h"); lcd.print(staHumid);
           staHumid += 0.01;       
           delay(50);        
        }      
      }

      while(digitalRead(interruptPin)==1){
        if(millis()-time_start1 > 1000){
           Serial.println("contious mode");
           lcd.setCursor(8, 2); lcd.print(humid+staHumid); lcd.setCursor(14, 2); lcd.print("h"); lcd.print(staHumid);
           staHumid -= 0.01;       
           delay(50);        
        }      
      }
      
      Serial.print("staHumid::");  Serial.println(staHumid);
      
      lcd.setCursor(8, 2); lcd.print("     "); lcd.setCursor(14, 2); lcd.print("      ");
      delay(500);
      lcd.setCursor(8, 2); lcd.print(humid+staHumid); lcd.setCursor(14, 2); lcd.print("h"); lcd.print(staHumid);
      delay(500);
      
    }
    while(staSet==2){
      Serial.println("out from setting mode**********************************");
      
      lcd.noBacklight();
      lcd.clear();
      EEPROM_write(0, String(staTemp));
      EEPROM_write(10, String(staHumid));
      Serial.println("SAVED .. ");
      
      staSet = 0;
      sta2 = 0;
      staAdd = 0;
    }
    delay(500);
    
   
    
  }
  
  
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
    
    delay(300);
    Serial.println("save mode");
    WiFiManager wifiManager;  
    //wifiManager.resetSettings();
    wifiManager.autoConnect("HT-A03");

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    
    lcd.clear();
    sta = 1;
    digitalWrite(LED,HIGH); //LED off
    digitalWrite(LED_savemode,HIGH); //LED off

    }
  //check connection  
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("connecte");
    digitalWrite(LED_online,HIGH); //LED on

    if(millis()-timeUpdateLog >= 10000){        //time update log firebase 1 min
      timeUpdateLog = millis();
      pubFirebase("A03",0);     //unit_id: A01,A02,A03,.. , float r: random val to make temp for another unit
//      pubFirebase("A01",0.8);
//      pubFirebase("A02",0.3);
//      pubFirebase("A04",-0.3);
    }
    
    if(millis()-timeUpdateData >= 60000){        //time update log firebase 1 min
      timeUpdateData = millis();
      logFirebase("A03",0);
//      logFirebase("A01",0.8);
//      logFirebase("A02",0.3);
//      logFirebase("A04",-0.3);
    }

    
  }
  else{
    Serial.println("NOT connected");
    digitalWrite(LED_online,LOW); //LED off
  }
  //--------------------------
   if(b==1 && sta2!=1){   
    if(millis()-timeBacklight >= 3000){        //time update log firebase 1 min
      timeBacklight = millis();
      lcd.noBacklight();
      b = 0;
    }
  }
  //---------------------------
  delay(300);   //time update current firebase 5sec
    
}



void handleInterrupt2() { 
    Serial.println("Interrupt Detected 2..............");
    if(sta2 != 0){
       staSet++;
       staAdd=0;
    }
    
    time_start = millis();
      while(digitalRead(interruptPin2)==1){
        if(millis()-time_start > 3000){
           Serial.println("setting mode");
           lcd.clear();
           lcd.setCursor(5, 0); lcd.print("Setting..."); 
           lcd.setCursor(0, 1); lcd.print("Temp  : ");
           lcd.setCursor(0, 2); lcd.print("Humid : ");
           
      
           lcd.backlight();
           sta2 = 1;

           Serial.print("staTEMP EEPROM:::");
           staTemp = EEPROM_read(0, 5).toFloat();
           Serial.println(staTemp);

           Serial.print("staHUMID EEPROM:::");
           staHumid = EEPROM_read(10, 5).toFloat();
           Serial.println(staHumid);   
               
           break;
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
  
  hic = dht.computeHeatIndex(temp+staTemp, humid+staHumid, false);

  if(hic < 27){       flag = "White";   water = "1/2"; train = 60; rest = 0; }
  else if(hic <= 32){ flag = "Green";   water = "1/2"; train = 50; rest = 10; }
  else if(hic <= 39){ flag = "Yellow";  water = "1";   train = 45; rest = 15; }
  else if(hic <= 51){ flag = "Red";     water = "1";   train = 30; rest = 30; }
  else {              flag = "Black";   water = "1";   train = 20; rest = 40; }


  Serial.print("Humidity: ");
  Serial.print(humid+staHumid);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temp+staTemp);
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
  //lcd.clear();
  Serial.println(NowString() + " "+ NowString2()); lcd.setCursor(0, 0); lcd.print(NowString2() + "     " + NowString());
  Serial.print("Temp: ");    Serial.print(temp+staTemp);   lcd.setCursor(0, 1); lcd.print("Temp  : "); lcd.print(temp+staTemp);
  Serial.print("\tHumid: "); Serial.print(humid+staHumid);  lcd.setCursor(0, 2); lcd.print("Humid : "); lcd.print(humid+staHumid); 
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
  Firebase.setFloat("ID/"+unit_id+"/Current/Temperature", temp+staTemp+r);
  Firebase.setFloat("ID/"+unit_id+"/Current/Humid", humid+staHumid+1.3*r);
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

     Serial.print("logFirebase............"); 
     String timeStamp = timeLog();
     Firebase.setString("ID/"+unit_id+"/Log/"+timeStamp+"/DateTime", timeLog());
     Firebase.setFloat("ID/"+unit_id+"/Log/"+timeStamp+"/Temperature", temp+staTemp+r);
     Firebase.setFloat("ID/"+unit_id+"/Log/"+timeStamp+"/Humid", humid+staHumid+1.3*r);
     Firebase.setFloat("ID/"+unit_id+"/Log/"+timeStamp+"/HID", hic+0.8*r);
     Firebase.setString("ID/"+unit_id+"/Log/"+timeStamp+"/Flag", flag);
     Firebase.setInt("ID/"+unit_id+"/Log/"+timeStamp+"/Train", train);
     Firebase.setInt("ID/"+unit_id+"/Log/"+timeStamp+"/Rest", rest);
     Firebase.setString("ID/"+unit_id+"/Log/"+timeStamp+"/Water", water);
     //Firebase.setInt("ID/"+unit_id+"/Log/"+timeStamp+"/Latitude", latitude);
     //Firebase.setString("ID/"+unit_id+"/Log"+timeStamp+"/Longtitude", longtitude); 
     Serial.println("Finish"); 
     
}

String EEPROM_read(int index, int length) {
  String text = "";
  char ch = 1;

  for (int i = index; (i < (index + length)) && ch; ++i) {
  if (ch = EEPROM.read(i)) {
  text.concat(ch);
  }
  }
  return text;
}

int EEPROM_write(int index, String text) {
  for (int i = index; i < text.length() + index; ++i) {
  EEPROM.write(i, text[i - index]);
  }
  EEPROM.write(index + text.length(), 0);
  EEPROM.commit();
  
  return text.length() + 1;
}

void handleInterruptAdd() { 
  state = !state;
  Serial.print("state: ");    Serial.print(state);

  if(state==1){
    if(sta2==1){
    if(staSet==0){
      time_start1 = millis();
      staTemp = staTemp+0.01;
      Serial.print("Temp: ");    Serial.print(temp+staTemp);   
      lcd.setCursor(8, 1); lcd.print(temp+staTemp); lcd.setCursor(14, 1); lcd.print("t"); lcd.print(staTemp);
  
    }
    if(staSet==1){
      staHumid = staHumid+0.01;
      Serial.print("\tHumid: "); Serial.print(humid+staHumid);  //lcd.setCursor(0, 2); lcd.print("Humid : "); lcd.print(humid+staHumid); 
      lcd.setCursor(8, 2); lcd.print(humid+staHumid); lcd.setCursor(14, 2); lcd.print("h"); lcd.print(staHumid);
    }
    
  }
  }
  
  
}
