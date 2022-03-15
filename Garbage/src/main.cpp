#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "WiFi.h"
#include "PubSubClient.h" //pio lib install "knolleary/PubSubClient"
#include "Keypad.h"
#include <Adafruit_PN532.h>
#include "HX711.h"

//MQTT
#define SSID          "NETGEAR68"
#define PWD           "excitedtuba713"
#define MQTT_SERVER   "192.168.1.2"
#define MQTT_PORT     1883
#define LED_PIN       2

WiFiClient espClient;
PubSubClient client(espClient);

//Staten
boolean reset;
boolean energie;
boolean actief;


//code 
int cinput[4];
int code[] = {-1,-1,-1,-1};


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

  if (messageTemp == "Reset escaperoom"){
    reset == true;
  }

  else if(messageTemp == "Groen"){
    energie == true;
  }
  else if(messageTemp == "Oranje"){
    energie == false;
  }
  else if(messageTemp == "Rood"){
    energie == false;
  }

  else if(messageTemp.indexOf("Wristband-code") > 0){
    code[0]= messageTemp.charAt(16);
    code[1]= messageTemp.charAt(17);
    code[2]= messageTemp.charAt(18);
  }
  else if(messageTemp.indexOf("Trein-code") > 0){
    code[3]= messageTemp.charAt(12);
    
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
      client.subscribe("trappenmaar/zone");
      client.subscribe("wristbands/3cijfers");
      client.subscribe("treingame/4decijfer");
      client.subscribe("controlpanel/reset");
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


int rest = 0; //Hoeveel rest klaar?
int pmd = 0; //Hoeveel pmd klaar?
int p_k = 0; //Hoeveel papier en karton klaar?
bool defGewicht;

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

//I2C multiplexer
void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  Serial.print(bus);
}

//Varia
#define Button_pin1 1
#define Button_pin2 2
#define Button_pin3 3
char* straf;

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
uint8_t uidLength = 7; 
uint8_t juisteWaardes1[4][7];
//uint8_t juisteWaardes[4][uidLength] = {{0x04, 0x0B, 0x43, 0x3A, 0xED, 0x4C, 0x81}, {0x04, 0xF2, 0x84, 0xA2, 0x2D, 0x4D, 0x80},
// {0x04, 0xF2, 0x84, 0xA2, 0x2D, 0x4D, 0x80}, {0x04, 0xEB, 0x83, 0xA2, 0x2D, 0x4D, 0x80}};
uint8_t juisteWaardes2[4][7];
uint8_t juisteWaardes3[4][7];

Adafruit_PN532 nfc1(1,2 );
Adafruit_PN532 nfc2(1,2 );
Adafruit_PN532 nfc3(1,2 );



void scanRFID1(){
  if(energie && actief){
    TCA9548A(0);
    uint8_t success;

    success = nfc1.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

    if (success) {

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
          rest++;
        }

        }



      }

      if(!juist){
       client.publish("trappenmaar/buffer",straf);
      }
      


  }

}}

void scanRFID2(){
   if(energie && actief){
    TCA9548A(1);
    uint8_t success;

    success = nfc2.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

    if (success) {

    //Check rfid
    bool juist = false;
      for(int j = 0;j<aantalVuilnis;j++){
        checkVuilnis = true;
        for(int i = 0;i<uidLength;i++){
          if( uid[i]!= juisteWaardes2[j][i]){
            checkVuilnis == false;
          }
        }

        if (checkVuilnis == true){
          juist = true; //Er is een juiste tag gevonden
          for(int k = 0;k<uidLength;k++){
          juisteWaardes2[j][k] == -1;
          pmd++;
        }

        }


      }

      if(!juist){
       client.publish("trappenmaar/buffer",straf);
      }
      


  }

}

}

void scanRFID3(){
   if(energie && actief){
    TCA9548A(2);
    uint8_t success;

    success = nfc3.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

    if (success) {

    //Check rfid
    bool juist = false;
      for(int j = 0;j<aantalVuilnis;j++){
        checkVuilnis = true;
        for(int i = 0;i<uidLength;i++){
          if( uid[i]!= juisteWaardes3[j][i]){
            checkVuilnis == false;
          }
        }

        if (checkVuilnis == true){
          juist = true; //Er is een juiste tag gevonden
          for(int k = 0;k<uidLength;k++){
          juisteWaardes3[j][k] == -1;
          p_k++;
        }

        }



      }

      if(!juist){
       client.publish("trappenmaar/buffer",straf);
      }
      


  }

}

}




void resetPuzzel(){
  setup();
  reset = false;

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
        client.publish("garbage/status","Garbage code is correct ingegeven");
      }

      else{
        client.publish("trappenmaar/buffer",straf);
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

void eindePuzzel(){

  //Tegen flikkeren
    if(bl ==false){
      lcd.backlight();
      bl = true;
    }

    

lcd.clear();
lcd.setCursor(2, 0);
lcd.print("Alles gesorteerd");
lcd.setCursor(2, 1);
lcd.print("Definitief gewicht:");
lcd.setCursor(0, 2);
lcd.print("Rest     PMD     P&K"); 
lcd.setCursor(0,3);
/*if (!defGewicht){
  lcd.print ...
} */


//Nog afwerken bro





}


void setup() {
  // lcd init
  lcd.init();
  lcd.backlight();
  codeTekst = false;
  c= 8;
  n = 0;

  defGewicht =false;



  //Serial monitor en I2C
  Serial.begin(115200);
  Wire.begin();

  //MQTT
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);

  //Buttons 
  attachInterrupt(digitalPinToInterrupt(Button_pin1), scanRFID1, RISING);
  attachInterrupt(digitalPinToInterrupt(Button_pin2), scanRFID2, RISING);
  attachInterrupt(digitalPinToInterrupt(Button_pin3), scanRFID3, RISING);

  //Overige
  char straf;

  //RFID
  TCA9548A(0);
  nfc1.begin();

  uint32_t versiondata1 = nfc1.getFirmwareVersion();
  if (! versiondata1) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata1>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata1>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata1>>8) & 0xFF, DEC);

  nfc1.SAMConfig();

  TCA9548A(1);
  nfc2.begin();

  uint32_t versiondata2 = nfc2.getFirmwareVersion();
  if (! versiondata2) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata2>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata2>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata2>>8) & 0xFF, DEC);

  nfc2.SAMConfig();

  TCA9548A(2);
  nfc3.begin();
  //Kan weg vanaf hier
  uint32_t versiondata3 = nfc3.getFirmwareVersion();
  if (! versiondata3) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata3>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata3>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata3>>8) & 0xFF, DEC);
  //Tot hier
  nfc3.SAMConfig();


  //Ready
  client.publish("controlpanel/status","Garbage Ready");
  Serial.println("Ready gestuurd");
}
  
void loop() {

  

  
  key = keypad.getKey(); //Vraag de input van de key op




  //MQTT
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000)
  {
    lastMsg = now;
  }
  

  if(reset){
    resetPuzzel();
  }

  else if(checkVuilnis){
    eindePuzzel();

  }

  else if(!energie){
    geenEnergie();
  }

  else if(energie && actief){
    puzzel();
    if(p_k == 3 && pmd == 3 && rest == 3){
      checkVuilnis = true;
    }
  }

  else if(energie){
    enkelEnergie();
  }
 
  
}


