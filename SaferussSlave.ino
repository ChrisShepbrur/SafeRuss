#include <Wire.h>

const int RELEVANT_DEVICES = 11;                   //Relevante enheter som slaven jobber med. 

int leds[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
boolean slaveState[RELEVANT_DEVICES];

volatile int vi;                                   //receiveEvent jobber med Interruption Handling
                                                   //er derfor volatile. 

int trueState = 1;
int i;

void setup() {
  Wire.begin(8);
  Wire.onReceive(receiveEvent);                    //Motta event fra Master Arduinoen
  Serial.begin(9600);

  for (i = 0; i < sizeof(leds) / 2; i++) {
    pinMode(leds[i], OUTPUT);
  }
}

void loop() {
  delay(100);

}
void receiveEvent(int n) {
  while (Wire.available()) {
    if (i >= sizeof(leds) /2) {                       
      i = 0;
    }
    vi = Wire.read();

    vi == trueState ? slaveState[i] = true : slaveState[i] = false;
    digitalWrite(leds[i], slaveState[i]);   
  }
  i++;
}

