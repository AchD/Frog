#ifndef __frog
#define __frog

/*  
    FROG - MQTT Client
   --------------------
   2018 by Achim Diedrichs 
 */

#include <Arduino.h>
#include "IPAddress.h"
#include "Client.h"
#include "Stream.h"

#define __MQTT_keepAlive 30
#define __MQTT_nextPing 27000
#define __MQTT_reSubscribe 5000
#define __MQTT_publishInterval 3000
#define __MQTT_rePublish 3000

#if defined(__AVR_ATmega168__) || defined(__AVR_ATtiny85__)
  #define __frog_max_topics 12
  #define __frog_max_topic_length 24
  #define __frog_max_stream 48
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__)
  #define __frog_max_topics 24
  #define __frog_max_topic_length 48
  #define __frog_max_stream 96
#elif defined(__AVR_ATmega1280__) 
  #define __frog_max_topics 64
  #define __frog_max_topic_length 64
  #define __frog_max_stream 128
#elif defined(__AVR_ATmega2560__)
  #define __frog_max_topics 64
  #define __frog_max_topic_length 64
  #define __frog_max_stream 128
#else
  #define __frog_max_topics 128
  #define __frog_max_topic_length 64
  #define __frog_max_stream 128
#endif

#define __CONNECT      1<<4
#define __CONNACK      2<<4
#define __PUBLISH      3<<4
#define __PUBACK       4<<4
#define __PUPREC       5<<4
#define __PUBREL       6<<4
#define __PUBCOMP      7<<4
#define __SUBSCRIBE    8<<4
#define __SUBACK       9<<4
#define __UNSUBSCRIBE 10<<4
#define __UNSUBACK    11<<4
#define __PINGREQ     12<<4
#define __PINGRESP    13<<4
#define __DISCONNECT  14<<4

#define __DB_flags_ACK 0b11000000
#define __DB_flags_SUB 0b00100000
#define __DB_flags_FLT 0b00010000
#define __DB_flags_TXT 0b00001000
#define __DB_flags_QoS 0b00000110
#define __DB_flags_RET 0b00000001

#define __CON_flags_auth 0b11000000
#define __CON_flags_Wret 0b00100000
#define __CON_flags_WQoS 0b00011000
#define __CON_flags_Wflg 0b00000100
#define __CON_flags_ClSe 0b00000010

class Frog {
  private:
    Client* FrogClient;
    IPAddress MQTT_Broker;
    int MQTT_Port;                         
    char *MQTT_ClientID, *MQTT_user, *MQTT_password;
    char* MQTT_url;
    boolean MQTT_version, MQTT_alive;
    unsigned long lastResponse, lastReconnect, publishInterval, rePub;
    struct DataBase {
      char* topic;
      byte flags;
      float *floatValue;
      float floatLastValue;
      boolean *boolValue;
      boolean boolLastValue;
    };
    DataBase DB_mqtt[__frog_max_topics];
    int DB_nextID, DB_actID;
    char *boolCharTRUE = "ON";
    char *boolCharFALSE = "OFF";
    char *boolValTRUE = "1";
    char *boolValFALSE = "0\0";
    char* DB_boolTypeTRUE(int ID);
    char* DB_boolTypeFALSE(int ID);
    char *floatString = "000000000000";
    uint8_t MQTT_stream[__frog_max_stream];
    int MQTT_streamLength, MQTT_streamPos, MQTT_streamAppendLength, MQTT_streamAppendPos, ID;
    boolean MQTT_pub;
    void connect();
    void connack();
    void publishClient();
    void publishServer();
    void pubrelClient();
    void pubcompClient();
    void pubackServer();
    void pubrecServer();
    void pubcompServer();
    void subscribeClient();
    void subackServer(); 
    void pingreqClient();
    void pingrespServer();
    int streamReadACK(int value);
    String streamReadString(int stringPos, int stringLength);
    void streamBuildFixedHeader(int ControlPacketType, int TypeFlag);
    void streamBuildMessage(char* streamArray); 
  public:
    Frog(Client& CLIENT, IPAddress BROKER, int PORT, char *CLIENTID);
    Frog(Client& CLIENT, char* BROKER, int PORT, char *CLIENTID);
    Frog (Client& CLIENT, IPAddress BROKER, int PORT, char *CLIENTID, char *USER, char *PWD);
    Frog (Client& CLIENT, char* BROKER, int PORT, char *CLIENTID, char *USER, char *PWD);
    boolean loop();
    boolean setPublish(char* topic, boolean* value);
    boolean setPublish(char* topic, boolean* value, byte QoS);
    boolean setPublish(char* topic, boolean* value, char* boolType);
    boolean setPublish(char* topic, boolean* value, byte QoS, char* boolType);
    boolean setPublish(char* topic, float* value);
    boolean setPublish(char* topic, float* value, byte QoS);
    boolean setSubscribe(char* topic, boolean* value);
    boolean setSubscribe(char* topic, boolean* value, byte QoS);
    boolean setSubscribe(char* topic, boolean* value, char* boolType);
    boolean setSubscribe(char* topic, boolean* value, byte QoS, char* boolType);
    boolean setSubscribe(char* topic, float* value);
    boolean setSubscribe(char* topic, float* value, byte QoS);
};
#endif


