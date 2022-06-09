//MQTT
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//RFID
#include <SPI.h>      
#include <MFRC522.h>  

#define RST_PIN        5          
#define SS_PIN         4 

//RFID LEDs
#define LED_PIN_RED    16
#define LED_PIN_GREEN  0
#define RFID_BUTTON_DOOR_OPEN 2
#define LED_PIN_LIGHT 15

// MQTT Broker
const char *mqtt_broker = "192.168.0.1";
const char *topic = "MQTT/Test";
const char *topic_rfid = "Serverraum/RFID";
const int mqtt_port = 1883;
MFRC522 mfrc522(SS_PIN, RST_PIN); 

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);

  SPI.begin();
  mfrc522.PCD_Init(); 
  // connecting to a WiFi network
  WiFi.begin("NicoMurat", "NicoMurat");
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect("pudding",NULL,NULL,"Ausfall/Bericht",1,true,"ESP 1 Ausgefallen")) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  // publish and subscribe
  client.publish(topic, "MQTT for RFID Ready!");
  

  pinMode(RFID_BUTTON_DOOR_OPEN, INPUT);
  pinMode(LED_PIN_RED, OUTPUT);
  pinMode(LED_PIN_GREEN, OUTPUT);
  pinMode(LED_PIN_LIGHT, OUTPUT);
  digitalWrite(LED_PIN_RED, LOW);
  digitalWrite(LED_PIN_GREEN, LOW);
  digitalWrite(LED_PIN_LIGHT, LOW);
}

void LIGHTON()
{
  digitalWrite(LED_PIN_LIGHT, HIGH);
}

void LIGHTOFF()
{
  digitalWrite(LED_PIN_LIGHT, LOW);
}

void TOGGLE_LED()
{
  if (digitalRead(LED_PIN_LIGHT)== HIGH)
  {
    LIGHTOFF();
  }
  else
  {
    LIGHTON();
  }
}

void accessGranted()
{
  Serial.println(" Access Granted ");
  Serial.println();
  digitalWrite(LED_PIN_GREEN, HIGH);
  delay(3000);
  digitalWrite(LED_PIN_GREEN, LOW);
  client.publish(topic_rfid, " Access Granted ");
}

void accessRefused()
{
   Serial.println(" Access Refused ");
   digitalWrite(LED_PIN_RED, HIGH);
   delay(3000);
   digitalWrite(LED_PIN_RED, LOW); 
   client.publish(topic_rfid, " Access Refused ");
}


void loop() {
  client.loop();

  int Btn_Door = digitalRead(RFID_BUTTON_DOOR_OPEN); 

  if (  mfrc522.PICC_IsNewCardPresent() &&  mfrc522.PICC_ReadCardSerial())
  {
    Serial.println();
    Serial.print(" UID tag  :");
    String content= "";
  
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
       Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
       Serial.print(mfrc522.uid.uidByte[i], HEX);
       content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
       content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    Serial.println();
    Serial.print(" PICC type: ");
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    if (content.substring(1) == "D9 B6 F0 94" || content.substring(1) == "B2 38 DB 0E")
    {
      accessGranted();
      LIGHTON(); 
    }
  
    else
    {
      accessRefused();
    }  
  }
  else if(Btn_Door == LOW)
  {
    accessGranted();
    TOGGLE_LED();
  }
  
}
