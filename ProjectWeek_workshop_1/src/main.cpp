#include <Arduino.h>
#include "die-shield.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Started");

  Die.begin();
  Die.show(FIVE);


}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Still alive");
  delay(1000);
  if(Die.isShaking()){
    Die.show(THREE);
  }
  else Die.show(ONE);
}