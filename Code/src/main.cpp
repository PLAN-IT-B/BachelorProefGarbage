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

WiFiClient espClient;
PubSubClient client(espClient);

//Staten
boolean reset = false;
boolean energie;
boolean actief;
boolean checkVuilnisTotaal;
boolean wachtOpGewicht;


//code 
int cinput[5];
int code[] = {-1,-1,-1,-1,-1}; 


long lastMsg = 0;
char msg[50];
int value = 0;

void callback(char *topic, byte *message, unsigned int length);

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

//Gewicht variabelen
int rest = 0; //Hoeveel rest klaar?
int pmd = 0; //Hoeveel pmd klaar?
int p_k = 0; //Hoeveel papier en karton klaar?
float restG; //gewicht rest def
int pmdG; //gewicht pmd def
int p_kG; //gewicht papier en karton def
int Rrest; //Gewicht afgerond eindresultaat
int Rpmd; //Gewicht afgerond eindresultaat
int Rp_k; //Gewicht afgerond eindresultaat

bool defGewicht = false;



//Keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {4, 19, 18,17}; // {18,5,17,6}connect to the row pinouts of the keypad
byte colPins[COLS] = {16,2,5}; //{4,0,2}connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char key;

//I2C multiplexer
void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  Serial.print(bus);
}

//Buttons en sound
#define Button_pin1 32
#define Button_pin2 25
#define Button_pin3 33
#define sound 23  


//Initialisatie weegschalen + variabelen voor check
HX711 scale, scale2,scale3;
float vorigGewicht;
float huidigGewicht;
int nummerWeegschaal;

//Hoeveel RFID's/vuilnisbak + variabele voor check
int aantalVuilnis = 4;
bool checkVuilnis;


//lcd
LiquidCrystal_I2C lcd(0x27,20,4);
int c; //variabele om plaats code te bepalen
boolean bl; //Variabele voor het flikkeren van backlight tegen te gaan
boolean codeTekst; //Staat er al tekst?

//RFID lijsten met juiste headers
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; 
uint8_t uidLength = 7; 
uint8_t juisteWaardes2[4][7] = {{0x04, 0xBF, 0x04, 0x82, 0x31, 0x4D, 0x84}, {0x04, 0xC7, 0x04, 0x82, 0x31, 0x4D, 0x84}, {0x04, 0xC3, 0x03, 0x82, 0x31, 0x4D, 0x84}, {0x04, 0xBB, 0x03, 0x82, 0x31, 0x4D, 0x84}};
uint8_t juisteWaardes1[4][7] = {{0x04, 0xB3, 0x03, 0x82, 0x31, 0x4D, 0x84}, { 0x04, 0xAB, 0x03, 0x82, 0x31, 0x4D, 0x84}, {0x04, 0xA3, 0x03, 0x82, 0x31, 0x4D, 0x84}, {0x04, 0x99, 0x02, 0x82, 0x31, 0x4D, 0x84}};
uint8_t juisteWaardes3[4][7] = {{0x04, 0xC8, 0x02, 0x82, 0x31, 0x4D, 0x84}, {0x04, 0xD0, 0x03, 0x82, 0x31, 0x4D, 0x84}, {0x04, 0xDA, 0x02, 0x82, 0x31, 0x4D, 0x84}, {0x04, 0xE3, 0x02, 0x82, 0x31, 0x4D, 0x84}};


Adafruit_PN532 nfc(4,0); // variabele om RFID aan te spreken



void setup() {
  // lcd init
  lcd.init();
  lcd.backlight();
  codeTekst = false;
  c= 8;
  

  

  defGewicht =false;
  checkVuilnisTotaal = false;

  //Button
  pinMode(Button_pin1, INPUT);
  pinMode(Button_pin2, INPUT);
  pinMode(Button_pin3, INPUT);

  //Scale1
  scale.begin(26,27);

  //apply the calibration
  scale.set_scale(207200);
 
  scale.tare();	//Reset the scale to 0



  //Scale 2
   scale2.begin(23,27);

  //apply the calibration
  scale2.set_scale(207200);

   
   scale2.tare();	//Reset the scale to 0*/

   

   //Scale3
   scale.begin(14,27);

   //apply the calibration
   scale.set_scale(207200);
 
   //initializing the tare. 
   scale.tare();	//Reset the scale to 0



  

  //Serial monitor en I2C
  Serial.begin(115200);
  Wire.begin();

  //MQTT
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);





  
  //RFID 1 initialiseren

  TCA9548A(7);
   nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    lcd.setCursor(0,0);
    lcd.print("RFID niet gevonden");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  

  //RFID 2 initialiseren
  TCA9548A(6);
   nfc.begin();

   versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
   lcd.setCursor(0,0);
    lcd.print("RFID niet gevonden");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  //RFID 3 initialiseren

  TCA9548A(5);
   nfc.begin();

   versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    lcd.setCursor(0,0);
    lcd.print("RFID niet gevonden");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();



}


//Reageren op inputs
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

  //Reset
  if (messageTemp == "Reset escaperoom"){
    reset = true;
  }

  //Energieniveau
  else if(messageTemp == "groen"){
    energie = true;
  }
  else if(messageTemp == "oranje"){
    energie = false;
  }
  else if(messageTemp == "rood"){
    energie = false;
  }

  //Ontvang codes
  else if(messageTemp.indexOf("Wristband-code") >= 0){
    code[0]= (messageTemp.charAt(15)-'0');
    code[1]= (messageTemp.charAt(16)-'0');
    code[2]= (messageTemp.charAt(17)-'0');
    code[3]= (messageTemp.charAt(18)-'0');
    Serial.println("Code ontvangen");
    for(int i = 0; i<4;i++){
      Serial.println(code[i]);
    }
  }
  else if(messageTemp.indexOf("Trein-code") >= 0){
    code[4]= (messageTemp.charAt(11)-'0');
    
  }

  //Startgeluid
  if (messageTemp == "Buzz lawaai om te starten"){
    tone(sound,NOTE_C3,2000,0);
    noTone(sound,0);
    
  }



}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    lcd.setCursor(0,0);
    lcd.print("Connecting ...");
    if (client.connect("GarbageESP"))
    {
      Serial.println("connected");
      // Subscribe
      client.subscribe("wristbands/#");
      client.subscribe("treingame/#");
      client.subscribe("controlpanel/reset");
      client.subscribe("TrappenMaar/zone");
      client.subscribe("garbage/eindcode");
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
    TCA9548A(7);
    uint8_t success = false;
    schrijfScannen();
   

    //Probeer te lezen voor 500ms
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,500);

    //Als er iets is gelezen
    if (success) {
      Serial.print("Succesvol gelezen: ");
      nfc.PrintHex(uid, uidLength);
    //Check of de RFID juist is
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
          
          juisteWaardes1[j][k] = 0;
        
        }
          rest++;
          
          Serial.println("correct!");
          Serial.println(rest);


          //Ga naar de Gewichttester staat
          codeTekst = false;
          wachtOpGewicht = true;
          nummerWeegschaal = 1;
        }



      }

      if(!juist){
       client.publish("TrappenMaar/buffer","grote fout");
       Serial.println("Fout");
       failureSound();
      }
      else(succesSound());
      
      


  }

}}

void scanRFID2(){ //Idem rfid 1
   if(energie && actief){
    Serial.println("Scanning ...");
    TCA9548A(6);
    uint8_t success = false;
    schrijfScannen();
   

   
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,500);

  
    if (success) {
      Serial.print("Succesvol gelezen: ");
      nfc.PrintHex(uid, uidLength);
    //Check rfid
    bool juist = false;
      for(int j = 0;j<aantalVuilnis;j++){
        checkVuilnis = true;
      
        for(int i = 0;i<uidLength;i++){
          if( uid[i]!= juisteWaardes2[j][i]){
            checkVuilnis = false;
          }
        }

        if (checkVuilnis == true){
          juist = true; //Er is een juiste tag gevonden
          for(int k = 0;k<uidLength;k++){
          
          juisteWaardes2[j][k] = 0;
        
        }
          pmd++;
          
          Serial.println("correct!");
          Serial.println(pmd);


          //Gewicht
          codeTekst = false;
          wachtOpGewicht = true;
          nummerWeegschaal = 2;
        }



      }

      if(!juist){
       client.publish("TrappenMaar/buffer","grote fout");
       Serial.println("Fout");
       failureSound();
      }
      else(succesSound());
      
      


  }

}}

void scanRFID3(){ //Idem rfid 1
   if(energie && actief){
    Serial.println("Scanning ...");
    TCA9548A(5);
    uint8_t success = false;
    schrijfScannen();
   

   
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,500);

  
    if (success) {
      Serial.print("Succesvol gelezen: ");
      nfc.PrintHex(uid, uidLength);
    //Check rfid
    bool juist = false;
      for(int j = 0;j<aantalVuilnis;j++){
        checkVuilnis = true;
      
        for(int i = 0;i<uidLength;i++){
          if( uid[i]!= juisteWaardes3[j][i]){
            checkVuilnis = false;
          }
        }

        if (checkVuilnis == true){
          juist = true; //Er is een juiste tag gevonden
          for(int k = 0;k<uidLength;k++){
          
          juisteWaardes3[j][k] = 0;
        
        }
          p_k++;
          
          Serial.println("correct!");
          Serial.println(p_k);


          //Gewicht
          codeTekst = false;
          wachtOpGewicht = true;
          nummerWeegschaal = 3;
        }



      }

      if(!juist){
       client.publish("TrappenMaar/buffer","grote fout");
       Serial.println("Fout");
       failureSound();
      }
      else(succesSound());
      
      


  }

}}




//Reset staat
void resetPuzzel(){
  ESP.restart();
 

}

//gewichtWachter staat
void gewichtWachter(){ 
    if (codeTekst == false){
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Gooi het vuilnis");
    lcd.setCursor(2,2);
    lcd.print("in de juiste bak");
    codeTekst =true;

    switch(nummerWeegschaal){ //Oorsprongkelijk gewicht
      case 1: vorigGewicht = scale.get_units(); 
      Serial.println(vorigGewicht);
      break;

      case 2: vorigGewicht = scale2.get_units(); 
      Serial.println(vorigGewicht);
      break;

      case 3: vorigGewicht = scale3.get_units(); 
      Serial.println(vorigGewicht);
      break;
    }
  }

    switch(nummerWeegschaal){ //nieuw gewicht
      case 1: huidigGewicht = scale.get_units(); 
      Serial.println(huidigGewicht);
      break;

      case 2: huidigGewicht = scale2.get_units(); 
      Serial.println(huidigGewicht);
      break;

      case 3: huidigGewicht = scale3.get_units(); 
      Serial.println(huidigGewicht);
      break;
    }

      if((huidigGewicht - vorigGewicht)>0.01){ //Is er meer dan 10g bij gekomen?
        //Verlaat deze staat
        wachtOpGewicht = false;
        codeTekst= false;
      }

      
  
    }
  

  



//Geen energie staat
void geenEnergie(){
lcd.noBacklight();
    bl = false;
    codeTekst = false;
}

//Energie zonder activatie staat
void enkelEnergie(){

  key = keypad.getKey(); //Vraag de input van de key op

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
    lcd.print("_____");
    codeTekst = true;
    }

    //Serial.print(key);
    
    //Als de knop wordt ingedrukt
    if(key!=NULL){
      
    //Als # (enter wordt ingedrukt)
    if(key =='#'){
      
      
      if(c ==13){ //De positie is het laatste cijfer
      boolean check = true; //Controleer of de code klopt
      for(int i = 0;i<5;i++){
        if(code[i]!=cinput[i]){
          check = false;
        }
      }

      if(check == true){ //Als de code klopt wordt de puzzel actief
        actief = true;
        client.publish("garbage/status","Garbage code is correct ingegeven");
        client.publish("Wristbands","Stop Wristbands");
        lcd.clear();
        lcd.setCursor(4,1);
        lcd.print("Code correct");
        succesSound();
        delay(500);
        lcd.clear();
        codeTekst = false;

      }
      

      else{ //Bestraffen en reset
        client.publish("TrappenMaar/buffer","grote fout");
        client.publish("TrappenMaar/buffer","grote fout");
        lcd.setCursor(8,2);
        lcd.print("_____");
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
      
     

      if(c<13){ //Vul het getal in en schuif 1 plaats op.
      lcd.setCursor(c,2);
      lcd.print(key);
      cinput[c-8]= key-'0';
      lcd.setCursor(c,2);
      c++;
      lcd.display();
      }
  
    }

  
  }
}

//Puzzel staat
void puzzel(){
   //Tegen flikkeren
    if(bl ==false){
      lcd.backlight();
      bl = true;
    }

    //Test scale
   // Serial.println(scale.get_units(), 3);

    //Serial.println("In puzzel");

    if (codeTekst == false){ //LCD instellen
    lcd.setCursor(0,0);
    lcd.print("Zoek vuilnis, scan");
    lcd.setCursor(0,1);
    lcd.print("in het juiste vak");
    lcd.setCursor(0,2);
    lcd.print("en gooi het in de");
    lcd.setCursor(0,3);
    lcd.print("correcte vuilnisbak");
    codeTekst = true;
    }


  //Check of knoppen zijn ingedrukt en lees de juiste scanner
  if(digitalRead(Button_pin1) == HIGH){
    scanRFID1();
  }
  if(digitalRead(Button_pin2) == HIGH){
    scanRFID2();
  }
  if(digitalRead(Button_pin3) == HIGH){
    scanRFID3();
  }

  if(rest == 3 && pmd == 3 && p_k == 4){ //Is al het vuilnis gevonden?
      checkVuilnisTotaal = true;

 }}

//Einde puzzel staat
void eindePuzzel(){

  //Tegen flikkeren
    if(bl ==false){
      lcd.backlight();
      bl = true;
    }

    


if (!defGewicht){ //1 maal het gewicht bepalen

//Lees gewicht

  restG = scale.get_units();
  pmdG = scale2.get_units();
  p_kG = scale3.get_units();
 
 //Maak het gewicht een int in gram
  Rrest = round(restG*1000);
  Rpmd = round(pmdG*1000);
  Rp_k = round(p_kG*1000);
 


 int eindcodeInt = (Rrest+pmdG+p_kG);
 String eindcodeString;
 if(eindcodeInt <999){
 eindcodeString = "0" + String(eindcodeInt,DEC);
 }
 else if(eindcodeInt <99){

   eindcodeString = "00" + String(eindcodeInt,DEC);
 }
 else{
   eindcodeString = String(eindcodeInt,DEC);
 }
 Serial.print(eindcodeString);
 const char* eindcode=eindcodeString.c_str();
 
 
  

  client.publish("garbage/eindcode",eindcode); 


  defGewicht = true;

} 

//print gewicht
if(codeTekst==false){
lcd.clear();
lcd.setCursor(2, 0);
lcd.print("Alles gesorteerd");
lcd.setCursor(1, 1);
lcd.print("Hoeveel in totaal?");
lcd.setCursor(0, 2);
lcd.print("PMD     Rest     P&K"); 
lcd.setCursor(3,3);
lcd.print("g");
lcd.setCursor(12,3);
lcd.print("g");
lcd.setCursor(19,3);
lcd.print("g");
codeTekst = true;

lcd.setCursor(0,3);
lcd.print(Rrest);
  
  
lcd.setCursor(9,3);
lcd.print(Rpmd);
  
lcd.setCursor(16,3);
lcd.print(Rp_k);
}


}



  
void loop() { //Check in welke staat het programma is

  //MQTT
  if (!client.connected())
  {
    reconnect();
    client.publish("controlpanel/status","Garbage Ready");
    Serial.println("Ready gestuurd");
  }
  
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000)
  {
    lastMsg = now;
  }
  

  if(reset){
    Serial.println("reset");
    resetPuzzel();
  }

  else if(!energie){
    geenEnergie();
  }

  else if(wachtOpGewicht){
    gewichtWachter();
  }

  else if(checkVuilnisTotaal){
    eindePuzzel();

  }



  else if(energie && actief){
    puzzel();
    
  }

  else if(energie){
    enkelEnergie();
  }
 
}


