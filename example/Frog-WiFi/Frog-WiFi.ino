#include <ESP8266WiFi.h>
#include <frog.h>

/* Programmbeispiel
 *  
 *  Arduino auf ESP8266 WiFi Board
 *  also MQTT Client.
 *  
 *  Im Beispiel der MQTT- Broker über Domain- Address.
 */

WiFiClient ethClient;
//MQTT Broker
Frog MQTT(ethClient, "mosquitto-broker.com", 1883, "Test_Client", "username", "Geheim");

unsigned long takt;
boolean dir, blinker;
float counter, maxcount;

void setup() {
  Serial.begin(115200);
  maxcount = 5000;
  WiFi.begin("ssid", "wlan-password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
  Serial.println("Starte Frog");
  //MQTT Variablen
  MQTT.setPublish("test/blinker", &blinker, "ON/OFF");
  MQTT.setPublish("test/counter", &counter, 1);  
  MQTT.setSubscribe("test/dir", &dir, 1); 
  MQTT.setSubscribe("test/max", &maxcount, 2);
}

void loop() {
  //MQTT Hintergrund
  MQTT.loop();
  //Arduino Program  
  if (takt<millis()) {
    Serial.print ("Maxwert: ");
    Serial.print(maxcount);
    blinker = !blinker;
    if (dir) {
      counter++;
      Serial.println("  -vor-");
    } else {
      counter--;
      Serial.println("  -zurueck-");
    }
    takt = millis() +1000;
  }
  if (counter >= maxcount) counter = 0;
  if (counter <= 0) counter = maxcount;
}

