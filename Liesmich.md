# Frog

Frog wurde in erster Linie dazu geschrieben um meine Arduino's unkompliziert mit MQTT an die Hausautomation anzubinden. Die digitalen oder analogen Werte, die per MQTT übertragen werden, kann man einfach mit den Variablen des Arduinos verknüpfen. Diese werden dann automatisch im Hintergrund von der Hausautomation beschrieben bzw. bei Wertänderung an die Hausautomation gesendet. Die Variablen können im Arduino Programm wie gewohnt benutzt werden. 
 
Da es auch schon mal vorkommen kann, dass eine Verbindung unterbrochen wird, überwacht Frog dies und meldet sich erneut am MQTT Broker an sobald die Verbindung wieder aufgebaut ist. (Subscribes werden automatisch aufgefrischt.)

Die Anzahl möglicher Variablenanbindungen und länge der Topics variiert je nach verwendetem Prozessor.


## Benutzung von Frog ##

Frog benötigt eine Client Verbindung via Ethernet oder WiFi.

## Einrichten der MQTT- Verbindung: ##

    Frog (client, Broker, Port, clientID, user*, password*)

Mit dem Aufruf des Konstruktor "Frog" werden die Verbindungsparameter übergeben. Diese Parameter sind im MQTT Broker (MQTT Server) festgelegt.


- client = Client der Ethernet- oder WiFi- Verbindung des Arduino.
- Broker = IP- Adresse (IPAddress) oder URL (const char) des MQTT Brokers.
- Port = Port des MQTT- Brokers, im Normalfall 1883.
- clientID = Eindeutiger Name des MQTT- Clients (const char).
- user = *Optional - Benutzername (const char) für die Authentifizierung am Broker.
- passwort = *Optional - Passwort (const char) zur Authentifizierung.

Falls der Broker keine Authentifizierung erwartet kannst du user und password weggelassen.  
## Einrichten der Variablenanbindung: ##

Frog kann sowohl Variablen vom Typ *boolean* als auch *float* verwenden. Diese Variablen werden wie üblich im Arduino deklariert und verwendet. Eine Variable kann entweder vom Client an den Broker gesendet werden (Publish) oder vom Broker empfangen werden (Subscribe).


    setPublish (topic, &value, QoS*, boolType*)
    setSubscribe (topic, &value, QoS*, boolType*)

Dem entsprechend benutzt du *setPublish* für Variablen die du an den Broker senden möchtest und *setSubscribe* für Variablen die Ihren Wert vom Broker erhalten.

- topic = MQTT- Variablenbezeichnung (const char) z.B. "home/floor/switch/lamp".
- &value = Adresse der verknüpften Arduino Variable. (Vorangehendes '&' nicht vergessen!)
- QoS = *Optional - "Quality of Service"  0,1 oder 2 (Wird bei QoS=0 weggelassen)
- BoolType = *Optional - Bei boolschen Variablen wird vom MQTT- Broker eine "1" oder "0" erwartet. Einige Hausautomationen verwenden auch "ON" und "OFF", dann schreibt man hier "ON/OFF" (const char).

## Hintergrundbearbeitung ##
Nachdem die Einrichtung vorgenommen wurde muss in jedem Durchlauf der Programmschleife "void loop()" der Frog - loop durchlaufen werden.

    loop()

Der Klassenname wurde hier jetzt nicht aufgeführt. Wie üblich wird dieser beim Konstruktoraufruf festgelegt und vor jeden Aufrauf der Klassenfunktion angegeben. 

Das alles kann man in den Beispielprogrammen sehen.

