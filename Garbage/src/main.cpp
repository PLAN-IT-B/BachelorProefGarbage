#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "WiFi.h"
#include "PubSubClient.h" //pio lib install "knolleary/PubSubClient"
#include "Keypad.h"
#include <Adafruit_PN532.h>
#include "HX711.h"
#include <Tone32.h>



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
boolean checkVuilnisTotaal;


//code 
int cinput[4];
int code[] = {1,1,1,1}; //Voorlopig pas aan naar -1, ...


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
int restG; //gewicht rest def
int pmdG; //gewicht pmd def
int p_kG; //gewicht papier en karton def
bool defGewicht = false;
int n; //Hoeveel vuil?

//Keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {18, 5, 17,16}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4,0,2}; //connect to the column pinouts of the keypad

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
#define Button_pin1 27
#define Button_pin2 26
#define Button_pin3 25
char* straf;
#define sound 14

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

Adafruit_PN532 nfc(33,32);

void schrijfScannen(){
  lcd.clear();
  lcd.setCursor(7,1);
  lcd.print("Scannen ...");
  codeTekst = false;
}

void failureSound(){
  tone(sound,NOTE_D5,100,0);
noTone(sound,0);
delay(50);
tone(sound,NOTE_D5,100,0);
noTone(sound,0);
delay(10);
tone(sound,NOTE_D5,300,0);
noTone(sound,0);
delay(10);
tone(sound,NOTE_C3,300,0);
noTone(sound,0);
}

void succesSound(){
tone(sound,NOTE_D5,100,0);
noTone(sound,0);
delay(50);
tone(sound,NOTE_D5,100,0);
noTone(sound,0);
delay(10);
tone(sound,NOTE_D5,300,0);
noTone(sound,0);
delay(10);
tone(sound,NOTE_A5,300,0);
noTone(sound,0);
}

void scanRFID1(){
  if(energie && actief){
    Serial.println("Scanning ...");
    TCA9548A(2);
    uint8_t success = false;
    schrijfScannen();
   

   
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,100);

  
    if (success) {
      Serial.print("Succesvol gelezen");
    //Check rfid
    bool juist = false;
      for(int j = 0;j<aantalVuilnis;j++){
        checkVuilnis = true;
        for(int i = 0;i<uidLength;i++){
          if( uid[i]!= juisteWaardes1[j][i]){
            checkVuilnis = false;
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
    TCA9548A(3);
    uint8_t success;

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,100);

    if (success) {
      Serial.print("Succesvol gelezen");

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
    TCA9548A(4);
    uint8_t success;
    nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

    if (success) {
      Serial.print("Succesvol gelezen");

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
        lcd.clear();
        lcd.setCursor(4,1);
        lcd.print("Code correct");
        succesSound();
        delay(500);
        lcd.clear();
        codeTekst = false;

      }
      

      else{
        client.publish("trappenmaar/buffer",straf);
        lcd.setCursor(8,2);
        lcd.print("____");
        c = 8;
        failureSound();
        






      }
    }

    
    }
   

    else if(key == '*'){ //Als * (terug) wordt ingevuld
        if(c>8){
        //Ga 1 terug, vervang het getal door _ en vervang de code door 0 (standaard getal in rij)
        c--;
        lcd.setCursor(c,2);
        lcd.print("_");
        cinput[c-8] = 0;
        }

      
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

    //Serial.println("In puzzel");

    if (codeTekst == false){ //LCD instellen
    lcd.setCursor(0,0);
    lcd.print("Zoek vuilnis, scan");
    lcd.setCursor(0,1);
    lcd.print("in het juiste bakje");
    lcd.setCursor(0,2);
    lcd.print("en gooi het in de");
    lcd.setCursor(0,3);
    lcd.print("correcte vuilnisbak");
    codeTekst = true;
    }



  if(digitalRead(Button_pin1) == HIGH){
    scanRFID1();
  }
 /* if(digitalRead(Button_pin2) == HIGH){
    scanRFID2();
  }
  if(digitalRead(Button_pin3) == HIGH){
    scanRFID3();
  }*/

 }

void eindePuzzel(){

  //Tegen flikkeren
    if(bl ==false){
      lcd.backlight();
      bl = true;
    }

    


lcd.setCursor(2, 0);
lcd.print("Alles gesorteerd");
lcd.setCursor(1, 1);
lcd.print("Definitief gewicht:");
lcd.setCursor(0, 2);
lcd.print("Rest     PMD     P&K"); 

if (!defGewicht){

//Lees gewicht




  lcd.setCursor(0,3);
  lcd.print(restG);
  lcd.setCursor(9,3);
  lcd.print(pmdG);
  lcd.setCursor(17,3);
  lcd.print(p_kG);


  defGewicht = true;
} 



}


void setup() {
  // lcd init
  lcd.init();
  lcd.backlight();
  codeTekst = false;
  c= 8;
  n = 4;

  defGewicht =false;
  checkVuilnisTotaal = false;

  //Button
  pinMode(Button_pin1, INPUT);
  pinMode(Button_pin2, INPUT);
  pinMode(Button_pin3, INPUT);





  //Serial monitor en I2C
  Serial.begin(115200);
  Wire.begin();

  //MQTT
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);



  //Overige
  char straf;

  //RFID
  TCA9548A(2);
   nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  


  /*TCA9548A(3);
   nfc.begin();

   versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  

  /*TCA9548A(2);
  nfc.begin();
  //Kan weg vanaf hier
  uint32_t versiondata3 = nfc.getFirmwareVersion();
  if (! versiondata3) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata3>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata3>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata3>>8) & 0xFF, DEC);
  //Tot hier
  nfc.SAMConfig();*/


  //Ready
  client.publish("controlpanel/status","GarbageReady");
  Serial.println("Ready gestuurd");
}
  
void loop() {

  
  energie = true; //Test
  
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

  else if(checkVuilnisTotaal){
    eindePuzzel();

  }

  else if(!energie){
    geenEnergie();
  }

  else if(energie && actief){
    puzzel();
    if(p_k == n && pmd == n && rest == n){
      lcd.clear();
      checkVuilnisTotaal = true;
    }
  }

  else if(energie){
    enkelEnergie();
  }
 
  
}


