//DHT11
#include <Adafruit_Sensor.h>  //Temperatursensor
#include <DHT.h>              //Temperatursensor
#include <DHT_U.h>            //Temperatursensor

#define DHTPIN         0

//Pin an dem das OUT Signal angeschlossen ist. 
float temp = 0.00;
float hum = 0.00;
DHT_Unified dht(DHTPIN, DHT11);

//MQTT
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// MQTT Broker
const char *mqtt_broker = "192.168.0.1";
const char *topic = "MQTT/Test";
const char *topic_temp = "Serverraum/Temperatur";
const char *topic_gas = "Serverraum/Gas";
const char *topic_luft = "Serverraum/Luftfeuchtigkeit";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();
  
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
      if (client.connect("pudding3",NULL,NULL,"Ausfall/Bericht",1,true,"ESP 3 Ausgefallen")) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  // publish and subscribe
  client.publish(topic, "MQTT für Mess-Sensoren Ready!");
}

void luftf() {
  sensors_event_t event;
  dht.humidity().getEvent(&event);    //Luftfeuchte messen und ausgeben
  hum = event.relative_humidity;
  Serial.print("Humidity: ");
  Serial.println( String(hum));
  client.publish(topic_luft, String(hum).c_str());
  delay(1000);
}

void temperatur() {
  sensors_event_t event;
  dht.temperature().getEvent(&event); //Temperatur messen und ausgeben
  temp = event.temperature;
  Serial.print("Temperature: ");
  Serial.println( String(temp));
  client.publish(topic_temp, String(temp).c_str());
  delay(1000);
}

void gas() {
  int val;
  val=analogRead(0);//Read Gas value from analog 0
  Serial.print("CO2 Gehalt: ");
  Serial.println(val,DEC);//Print the value to serial port
  client.publish(topic_gas, String(val).c_str());
  delay(1000);
}

void loop() {
  luftf();
  temperatur();
  gas();
}
