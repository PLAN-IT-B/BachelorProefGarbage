#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "WiFi.h"
#include "PubSubClient.h" //pio lib install "knolleary/PubSubClient"
#include "Keypad.h"
#include <Adafruit_PN532.h>

//MQTT
#define SSID          "NETGEAR68"
#define PWD           "excitedtuba713"
#define MQTT_SERVER   "192.168.1.20"
#define MQTT_PORT     1883
#define LED_PIN       2

WiFiClient espClient;
PubSubClient client(espClient);


long lastMsg = 0;
char msg[50];
int value = 0;


void setup_wifi()
{
  delay(10);
  Serial.println("Connecting to WiFi..");

  WiFi.begin(SSID, PWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
   
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Garbage"))
    {
      Serial.println("connected");
      // Subscribe
      client.subscribe("tag/#");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


//Staten
boolean reset = false;
boolean energie = true;
boolean actief = false;

//test
int n = 0;

//Keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {2, 3, 4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9,7,8}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char key;

//Varia
#define Button_pin1 1
#define Button_pin2 2
#define Button_pin3 3

//Hoeveel vuilnis in 1 vuilbak en check rfid
int aantalVuilnis;
bool checkVuilnis;


//lcd
LiquidCrystal_I2C lcd(0x27,20,4);
int c;
boolean bl;
boolean codeTekst;

//RFID
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; 
const uint8_t uidLength = 7; 
uint8_t juisteWaardes1[4][uidLength];
//uint8_t juisteWaardes[4][uidLength] = {{0x04, 0x0B, 0x43, 0x3A, 0xED, 0x4C, 0x81}, {0x04, 0xF2, 0x84, 0xA2, 0x2D, 0x4D, 0x80},
// {0x04, 0xF2, 0x84, 0xA2, 0x2D, 0x4D, 0x80}, {0x04, 0xEB, 0x83, 0xA2, 0x2D, 0x4D, 0x80}};
uint8_t juisteWaardes2[4][uidLength];
uint8_t juisteWaardes3[4][uidLength];

Adafruit_PN532 nfc1(pin1,pin2 );



void scanRFID1(){
  if(energie && actief){
    



    //Check rfid
    bool juist = false;
      for(int j = 0;j<aantalVuilnis;j++){
        checkVuilnis = true;
        for(int i = 0;i<uidLength;i++){
          if( uid[i]!= juisteWaardes1[j][i]){
            checkVuilnis == false;
          }
        }

        if (checkVuilnis == true){
          juist = true; //Er is een juiste tag gevonden
          for(int k = 0;k<uidLength;k++){
          juisteWaardes1[j][k] == -1;
        }

        }



      }

      if(!juist){
       //geef fout signaal
      }
      


  }

}

void scanRFID2(){

}

void scanRFID3(){

}



//code 
int cinput[4];
int code[] = {-1,-1,-1,-1};



void resetPuzzel(){
  setup();

}

void geenEnergie(){
lcd.noBacklight();
    bl = false;
    codeTekst = false;
}

void enkelEnergie(){
  //Tegen flikkeren
    if(bl ==false){
      lcd.backlight();
      bl = true;
    }

    //Check of het beginscherm er op staat, anders zet het er op
    if(codeTekst == false){
    lcd.setCursor(2,0);
    lcd.print("Voer de code in:");
    lcd.setCursor(8,2);
    lcd.print("____");
    codeTekst = true;
    }

    //Serial.print(key);
    
    //Als de knop wordt ingedrukt
    if(key!=NULL){
      
    //Als # (enter wordt ingedrukt)
    if(key =='#'){
      
      
      if(c ==12){ //De positie is het laatste cijfer
      boolean check = true; //Controleer of de code klopt
      for(int i = 0;i<4;i++){
        if(code[i]!=cinput[i]){
          check = false;
        }
      }

      if(check == true){ //Als de code klopt wordt de puzzel actief
        actief = true;
        //Stuur bericht
      }

      else{//Foute code stuur foutsignaal
        lcd.setCursor(8,2);
        lcd.print("____");
        c = 8;
        






      }
    }

    
    }
   

    else if(key == '*'){ //Als * (terug) wordt ingevuld
      
        //Ga 1 terug, vervang het getal door _ en vervang de code door 0 (standaard getal in rij)
        c--;
        lcd.setCursor(c,2);
        lcd.print("_");
        cinput[c-8] = 0;


      
    }
  
    
    else{ //Als er iets anders (cijfer) wordt ingedrukt
      Serial.println("Cijfer");
      Serial.println(c);

      if(c<12){ //Vul het getal in en schuif 1 plaats op.
      lcd.setCursor(c,2);
      lcd.print(key);
      cinput[c-8]= key-'0';
      lcd.setCursor(c,2);
      c++;
      lcd.display();
      }
  
    }

  key == NULL; //Reset het key signaal
  }
}


void puzzel(){
   //Tegen flikkeren
    if(bl ==false){
      lcd.backlight();
      bl = true;
    }

    //Hier moet de lcd nog aangestuurd worden (gewicht)

 }

void setup() {
  // lcd init
  lcd.init();
  lcd.backlight();
  codeTekst = false;
  c= 8;
  n = 0;

  //Serial monitor
  Serial.begin(115200);

  //MQTT
  /*setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);*/

  //Buttons 
  attachInterrupt(digitalPinToInterrupt(Button_pin1), scanRFID1, RISING);
  attachInterrupt(digitalPinToInterrupt(Button_pin2), scanRFID2, RISING);
  attachInterrupt(digitalPinToInterrupt(Button_pin3), scanRFID3, RISING);

  
}
  
void loop() {

  energie = true;
  reset = false;

  /*Serial.println(reset);
  Serial.println(energie);
  Serial.println(actief);
  Serial.println();*/

  //key 
  //key = keypad.getKey(); //Vraag de input van de key op




  //MQTT
 /* if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000)
  {
    lastMsg = now;
  }*/
  

  if(reset){
    resetPuzzel();
  }

  else if(!energie){
    geenEnergie();
  }

  else if(energie && actief){
    puzzel();
  }

  else if(energie){
    enkelEnergie();
  }
 
  
  
}


