#include <Ethernet.h>
#include <frog.h>

/* Programmbeispiel
 *  
 *  Arduino mit Ethernetshield (z.B. W5100)
 *  also MQTT Client.
 *  
 *  Im Beispiel der MQTT- Broker mit lokaler IP-Adresse.
 */

IPAddress ip(192, 168, 1, 177);
IPAddress server(192, 168, 1, 222);
EthernetClient ethClient;
Frog MQTT(ethClient, server, 1883, "Test-Client", "username", "Geheim");
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };


unsigned long takt;
boolean dir, blinker;
float counter, maxcount;

void setup() {
  Serial.begin(115200);
  maxcount = 100;
  Ethernet.begin(mac, ip);
  Serial.println("Starte Frog");
  MQTT.setPublish("test/blinker", &blinker, 2, "ON/OFF");
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
      Serial.println("  -vorwärts-");
    } else {
      counter--;
      Serial.println("  -rückwärts-");
    }
    takt = millis() +1000;
  }
  if (counter >= maxcount) counter = 0;
  if (counter <= 0) counter = maxcount;
}

