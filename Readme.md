# Frog

At first, Frog was written to easy bind my Arduino's per MQTT to my Smarthome software. The digital or analog values transmitted via MQTT can be easily linked to the variables of the Arduino. These are automatically described in the background by the home automation system or will be sent when the value changes. The variables can be used as usual in the Arduino program.

If sometimes a connection is interrupted, Frog monitors this and binds to the MQTT broker again as the connection is reestablished. (Subscribes are automatically refreshed.)

The number of connections and length of topics varies, depending on the used processor.


## Use of Frog ##

Frog uses a Client connection via Ethernet or WiFi.

## Setup the MQTT- connection: ##

    Frog (client, Broker, Port, clientID, user*, password*)

Calling the constructor "Frog" transfers the connection parameters. These parameters are set in the MQTT Broker (MQTT Server).


- client = Client of Ethernet- or WiFi- connection of the Arduino.
- Broker = IP- address (IPAddress) or URL (const char) of the MQTT Broker.
- Port = Port of the MQTT- Broker, normally 1883.
- clientID = Unique name of the MQTT- client (const char).
- user = *optional - Username (const char) for authentification at Broker.
- passwort = *optional - Password (const char) for authentification.

If Broker don't needs an authentification, you can omit user and password.  
## Setup variables connection: ##

Frog can use both *boolean* and *float* variables. These variables are declared and used in the Arduino as usual. A variable can either be sent by the client to the broker (Publish) or received by the broker (Subscribe).


    setPublish (topic, &value, QoS*, boolType*)
    setSubscribe (topic, &value, QoS*, boolType*)

Accordingly, you use *setPublish* for variables you want to send to the broker and *setSubscribe* for variables you want to receive from the broker.

- topic = MQTT- variable name (const char) for example "home/floor/switch/lamp".
- &value = Address of the linked Arduino variable. (Don't forget previous '&'!)
- QoS = *Optional - "Quality of Service"  0,1 or 2 (Is omitted at QoS=0)
- BoolType = oOptional - For Boolean variables, the MQTT broker expects a "1" or "0". Some home automation systems also use "ON" and "OFF", then write "ON / OFF" (const char) here.

## Background processing ##

After setup, the Frog - loop must be run in each program loop "void loop ()".

    loop()

The class name has not been listed here. As usual, it is set at the constructor call and specified before each class function call. All this can be seen in the example programs.
