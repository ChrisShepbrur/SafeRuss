/*
 * Led lampene vil lyse dersom den ikke finner tilhoerende addresse
 * Led lampene vil slutte aa lyse dersom tilhoerende addresse er innenfor HC-05 sin rekkevidde
 * 
 * Master arduinoen holder styr paa 9 ulike led, mens slaven holder paa de resterende 11. 
 * Inter-intergrated circuit (I2C) forbindelse mellom arduinoene. 
 * 
 */
 
#include <SoftwareSerial.h>                   //for tilkobling med hc05 modulen
#include <Wire.h>                             //For I2C kommunikasjon mellom de to arduinoene. 

SoftwareSerial hc05(11, 12); //RX, TX

const int RELEVANT_DEVICES = 20;              //Antall enheter som det er plass til paa tavla. 
                                              //Bluetooth addressene er gitt i dette formatet. 
String devices[RELEVANT_DEVICES] = {
                                    "", "", "+INQ:4474:6C:306DAF", "", "",
                                    "", "", "", "", "",
                                    "", "", "+INQ:F40E:22:ADBF80", "", ""
                                    };
                                    
int leds[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};    

boolean absoluteState[RELEVANT_DEVICES];      //tilstandsarray for alle led
boolean masterState[9];                       //tilstandsarray for led her
boolean slaveState[11];                       //tilstandsarray for led hos slaven. 

boolean tempState[RELEVANT_DEVICES];          //tilstandsarray som vi jobber med i koden, som forandres
                                              //dersom modulen finner en match

int i;

unsigned long current = millis();
unsigned long prev = 0;

const int addrLength = 19;                    //lengden paa bluetooth addressa 
String found;

void setup() {
 Wire.begin(8);                               //I2C kommunikasjon paa en vilkaarlig kanal (8). 
 Serial.begin(9600);

 //sizeof gir bytelengde, hver plass i int arrayet tilsvarer to bytes, derfor deler vi paa to. 
 for (i = 0; i < sizeof(leds) / 2; i++) { 
  pinMode(leds[i], OUTPUT);
 }

 //Av default skal tempState vere true pga den skal endres til false dersom noen er i naerheten
 //og dermed slukke tilhoerende led. 
 for (i = 0; i < RELEVANT_DEVICES; i++) {
  tempState[i] = true;
 }

 //Setter pin 34 til modulen til hoy for aa sette den i AT Command Mode
 pinMode(13, OUTPUT); digitalWrite(13, HIGH); 
 hc05.begin(38400);                          //default baud rate til modulen.


 /* 
  *  Dette er kommandoer som maa gjores for aller forste gang 
  *  for aa sette opp modulen i riktig modus.
 
 hc05.write("AT+ROLE=1\r\n");         //Setter modusen til Master. (Slave er default)
 delay(50);
 hc05.write("AT+CMODE=1\r\n");        //bestemmer connection mode til tilkobling til alle. 
 delay(50);
 
 hc05.write("AT+IAC=9e8b33\r\n");     //bestemmer inquire access code
 delay(50);
 */
 hc05.write("AT+INIT\r\n");           //initaliserer modulen og gjor den klar til inquiry
 delay(50);
 
 hc05.write("AT+INQM=1,20,48\r\n");   //setter inquiry access modus til 1 = rssi, antall responser 
                                      //og for hvor lenge. (48.1.28 = 60 sekunder)
 delay(50);
 
}

void loop() {
    
  while (hc05.available()) {
    hc05.write("AT+INQ\r\n");                 //Begynner med inquiring av enheter i naerheten. 
    char btAddr = char(hc05.read());          
    if (found.length() < addrLength) {        
      if (btAddr == '+') {                    //Unngaar 90% av garbage data som kommer ved aa sjekke
        found = btAddr;
        
      } else {
        found = found + btAddr;
      }
    } else {
      for(int x = 0; x < RELEVANT_DEVICES; x++){
        if(found.equals(devices[x])){
          tempState[x] = false;          
        }
      }
      //Serial.println(found);
      found = "";
    }
  } 
  
  delay(50);                                  //Delay her for aa hindre kun garbage. 
  refreshLeds(4000);
  delay(50);                                  //Delay her for at refreshLeds skal faa tid til aa gjore seg ferdig.
}

//Oppdaterer alle lyspaerene i ett gitt tidsintervall. 
void refreshLeds(int interval) {
  if ((millis() - prev) > interval) {
    for (i = 0; i < sizeof(absoluteState); i++) {     
      absoluteState[i] = tempState[i];        //Skriver tempState sine verdier inn til den fullverdige arrayen
      digitalWrite(leds[i], absoluteState[i]);
    }
    
    for (i = 0; i < sizeof(masterState); i++) {
      masterState[i] = absoluteState[i];
    }

    int transmission; 
    
    //Sender tilstandsverdiene for de resterende lysene til slavearduinoen. Begynner der den andre loekka slutta.
    for (i = sizeof(masterState); i < sizeof(absoluteState); i++) {
      slaveState[i] = absoluteState[i];
      Wire.beginTransmission(8);
      Wire.write(slaveState[i] ? transmission = 1 : transmission = 0);
      Wire.endTransmission();  
      
    }

    //resetter verdiene i tempState. True verdi gjoer at lampene vil lyse.
    for (i = 0; i < sizeof(tempState); i++) {
      tempState[i] = true;
    }
    prev = millis();
  }
  hc05.flush();                              //Sletter cache data i Serial. 
}

