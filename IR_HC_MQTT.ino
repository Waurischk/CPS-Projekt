//MQTT
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *mqtt_broker = "192.168.0.1";
const char *topic = "MQTT/Test";
const char *topic_IR = "Serverraum/Infrarot";
const char *topic_Licht = "Serverraum/Licht";
const char *topic_HC = "Serverraum/Ultraschall";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

//Infrarot
#define whiteLED 5
const int sensorPin = 3;     // the number of the sensor pin
const int ledPin =  13;      // the number of the LED pin
int sensorState = 2;         // variable for reading the sensor status

//Ultraschall
const int trigPin = 14;
const int echoPin = 16;
long dauer;
float abstand;

void setup() {
  Serial.begin(115200);

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
      if (client.connect("pudding2",NULL,NULL,"Ausfall/Bericht",1,true,"ESP 2 Ausgefallen")) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }  
  client.publish(topic, "MQTT for Anwesenheitsmeldungen Ready!");
  
  //Infrarot
  pinMode(ledPin, OUTPUT);      
  pinMode(sensorPin, INPUT); 
  pinMode(whiteLED, OUTPUT);

  //Ultraschall
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void ultraschall() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  dauer = pulseIn(echoPin, HIGH);
  abstand = dauer * 0.034 / 2;

  Serial.print("Abstand zum Objekt = ");
  Serial.print(abstand);
  Serial.println(" cm");
  if(abstand <= 1500) {
    client.publish(topic_HC, "Jemand ist in der nähe!");
  }
  else {
    client.publish(topic_HC, "Niemand ist in der nähe!");
  }
  delay(1000);
}

void infrarot() {

  int sensorStateValue = digitalRead(sensorState);
  Serial.println(sensorStateValue);
  delay(5000);
  // if it is, the sensorState is HIGH:
  if (sensorStateValue == 0) {     
     digitalWrite(ledPin, HIGH);
     client.publish(topic_IR, "Bewegung erkannt!");  
     delay(1000);
     if (digitalRead(whiteLED) == LOW) {
      digitalWrite(whiteLED, HIGH);
      client.publish(topic_Licht, "Licht an.");
     }
     else {
      digitalWrite(whiteLED, LOW);
      client.publish(topic_Licht, "Licht aus.");
     }
  } 
  else {
     digitalWrite(ledPin, LOW); 
     client.publish(topic_IR, "Keine Bewegung erkannt!");  
     delay(1000);
  }
}
void loop(){
  infrarot();
  ultraschall();
}
