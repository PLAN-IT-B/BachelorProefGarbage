#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "WiFi.h"
#include "PubSubClient.h" //pio lib install "knolleary/PubSubClient"
#include "Keypad.h"

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
byte colPins[COLS] = {6,7,8}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


//lcd
LiquidCrystal_I2C lcd(0x27,20,4);
int c;
boolean bl;
boolean codeTekst;



//code
int cinput[4];
int code[4];

void setup() {
  // lcd init
  lcd.init();
  lcd.backlight();
  codeTekst = false;
  c= 8;

  
  
  

  //Serial monitor
  Serial.begin(115200);

  //MQTT
  /*setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);*/

  
}
  

  
  
  


void loop() {

  //key 
  
  if(n==0){
  char key = keypad.getKey();
  
  }
  
  

 

//MQTT
  /*if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000)
  {
    lastMsg = now;
  }
  */
  
  if(reset == 1){
    //No energy
  }
  else if(!energie){
    lcd.noBacklight();
    bl = false;
    codeTekst = false;
    


  }

  else if(energie && actief){

    //Tegen flikkeren
    if(bl ==false){
      lcd.backlight();
      bl = true;
    }

  }

  else if(energie){

    //Tegen flikkeren
    if(bl ==false){
      lcd.backlight();
      bl = true;
    }

    
    if(codeTekst == false){
    lcd.setCursor(2,0);
    lcd.print("Voer de code in:");
    lcd.setCursor(8,2);
    lcd.print("____");
    codeTekst = true;
    }

    //Serial.print(key);
    
    if(key!=NULL){
      
      
    if(key =='#'){
      

      if(c ==11){
      boolean check = true;
      for(int i = 0;i<4;i++){
        if(code[i]!=cinput[i]){
          check = false;
        }
      }

      if(check == true){
        actief = true;
      }
      
    }

    
    }
   

    else if(key == '*'){
      Serial.println("back");
      
        c--;
        lcd.setCursor(c,2);
        lcd.print("_");
        cinput[c-8] = 0;
        
        


      
      
    }
  
    
    else{
      Serial.println("Cijfer");
      Serial.println(c);

      if(c<11){
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
}




