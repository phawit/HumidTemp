const int interruptPin = 0; //GPIO 0 (Flash Button) 
const int LED=2; //On board blue LED 
long time_start,time_stop;
int sta=1;
 
void setup() { 
  Serial.begin(115200); 
  pinMode(LED,OUTPUT); 
  pinMode(interruptPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING); 
  digitalWrite(LED,HIGH); //LED off 
} 
 
void loop() 
{   
   

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
