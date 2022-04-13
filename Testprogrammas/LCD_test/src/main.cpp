#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

void setup() {
  Serial.begin (115200);
  Serial.print("begin search");
  Wire.begin();
  byte count = 0;
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);  // maybe unneeded?
      } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");


  lcd.init();
  lcd.clear();         
  lcd.backlight();

  lcd.setCursor(6,1);  //(X,Y) = set cursor to x'th character of Y'th line.
  lcd.print("No power!");

  lcd.setCursor(3,2);
  lcd.print("Generate power!");

  

}



void loop() {
  // put your main code here, to run repeatedly:
}