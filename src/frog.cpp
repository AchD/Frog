#include "Frog.h"
#include "Arduino.h"

Frog::Frog (Client& CLIENT, IPAddress BROKER, int PORT, char *CLIENTID) {
  Frog (CLIENT, BROKER, PORT, CLIENTID, NULL, NULL);
}

Frog::Frog (Client& CLIENT, char *BROKER, int PORT, char *CLIENTID) {
  Frog (CLIENT, BROKER, PORT, CLIENTID, NULL, NULL);
}

Frog::Frog (Client& CLIENT, IPAddress BROKER, int PORT, char *CLIENTID, char *USER, char *PWD) {
  this->FrogClient = &CLIENT;
  MQTT_Broker = BROKER;
  MQTT_Port = PORT;
  MQTT_ClientID = CLIENTID;
  MQTT_user =  USER;
  MQTT_password = PWD;
}

Frog::Frog (Client& CLIENT, char *BROKER, int PORT, char *CLIENTID, char *USER, char *PWD) {
  this->FrogClient = &CLIENT;
  MQTT_url = BROKER;
  MQTT_Port = PORT;
  MQTT_ClientID = CLIENTID;
  MQTT_user =  USER;
  MQTT_password = PWD;
}

boolean Frog::loop() {
  if (!FrogClient->connected()) {
    MQTT_alive=false;
    FrogClient->stop();
  }
  memset(MQTT_stream,0,__frog_max_stream);
  MQTT_streamLength=0;
  MQTT_streamPos = 0;
  if(FrogClient->available() > 0) {
    MQTT_alive=true;    
    while (FrogClient->available() > 0) {
        MQTT_stream[MQTT_streamLength++] = FrogClient->read();
      }
    MQTT_streamAppendLength = MQTT_streamLength;
    MQTT_streamAppendPos = MQTT_streamLength;
    if (MQTT_streamLength > 0) {
    while (MQTT_streamPos < MQTT_streamLength) {
      byte ControlPacketType = byte(MQTT_stream[MQTT_streamPos] & 0xf0);
      switch (ControlPacketType) {
        case __CONNACK:
          connack();
          break;   
        case __PUBLISH:
          publishServer();
          break;
        case __PUBACK:
          pubackServer();
          break;
        case __PUPREC:
          pubrelClient();
          break;
        case __PUBREL:
          pubcompClient();
          break;
        case __PUBCOMP:
          pubcompServer();
          break;
        case __SUBACK:
          subackServer();
          break;
        case __PINGRESP:
          pingrespServer();
          break;
        }
        MQTT_streamPos = MQTT_streamPos + int(MQTT_stream[MQTT_streamPos+1]) + 2;
      }
      lastResponse = millis() + __MQTT_nextPing;
    }
      if (MQTT_streamAppendLength > MQTT_streamLength) {
        MQTT_streamAppendPos = MQTT_streamLength;
        while (MQTT_streamAppendPos < MQTT_streamAppendLength) {
          FrogClient->write(MQTT_stream[MQTT_streamAppendPos++]);
        }
      }
  } else {
    memset(MQTT_stream,0,__frog_max_stream);
    MQTT_streamLength=0;
    MQTT_streamPos = 0;
    if (MQTT_alive) {
      if (ID >= DB_nextID) {
        if (publishInterval < millis()) {
          publishInterval = millis() + __MQTT_publishInterval;
          MQTT_pub = true;
        } else {
          MQTT_pub = false;
        }
        ID = 0;
      }
      if ((DB_mqtt[ID].flags & __DB_flags_SUB) == 0) {
        if (((DB_mqtt[ID].flags & __DB_flags_ACK) == 0x00)) { 
           publishClient();  
          rePub = millis() + __MQTT_rePublish;
        } else if  ((!((DB_mqtt[ID].flags & __DB_flags_ACK) == 0xc0)) & (rePub <= millis())) {
          rePub = millis() + __MQTT_rePublish;
        } else if ((DB_mqtt[ID].flags & __DB_flags_FLT) == 0) {
          if (!(DB_mqtt[ID].boolLastValue == *DB_mqtt[ID].boolValue)) {
            publishClient();
          }
        } else {
          if (MQTT_pub) {
            if (!(DB_mqtt[ID].floatLastValue == *DB_mqtt[ID].floatValue)) {
              publishClient();        
            }
          }
        }    
      } else {
        if (!((DB_mqtt[ID].flags & __DB_flags_ACK) == 0xc0)) {
          subscribeClient();
        }
      }
    } 
    ID++;
    if ((lastReconnect <= millis()) & (!MQTT_alive)) {
      lastReconnect = millis() + __MQTT_reSubscribe;
      connect();
    }
  }
  if (lastResponse < millis()) { 
    pingreqClient();
    lastResponse = millis() + 3000;
  }
  return MQTT_alive;
}

boolean Frog::setPublish(char* topic, boolean* value) {
  setPublish(topic, value, 0, "");
}

boolean Frog::setPublish(char* topic, boolean* value, byte QoS) {
  setPublish(topic, value, QoS, "");
}

boolean Frog::setPublish(char* topic, boolean* value, char* boolType) {
  setPublish(topic, value, 0, boolType);
}

boolean Frog::setPublish(char* topic, boolean* value, byte QoS, char* boolType) {
  if ((DB_nextID<=__frog_max_topics) & (strlen(topic) <= __frog_max_topic_length)){
    DB_mqtt[DB_nextID].topic = topic;
    DB_mqtt[DB_nextID].boolValue = value;
    DB_mqtt[DB_nextID].flags = (QoS<<1)& __DB_flags_QoS;
    if (boolType == "ON/OFF") DB_mqtt[DB_nextID].flags |= __DB_flags_TXT;
    DB_nextID++;
    return true;
  } else {
    return false;
  }
}

boolean Frog::setPublish(char* topic, float* value) {
  setPublish(topic, value, 0);
}

boolean Frog::setPublish(char* topic, float* value, byte QoS) {
  if ((DB_nextID<=__frog_max_topics) & (strlen(topic) <= __frog_max_topic_length)){
    DB_mqtt[DB_nextID].topic = topic;
    DB_mqtt[DB_nextID].floatValue = value;
    DB_mqtt[DB_nextID].flags = (((QoS<<1) & __DB_flags_QoS) | __DB_flags_FLT);
    DB_nextID++;
    return true;
  } else {
    return false;
  }
}

boolean Frog::setSubscribe(char* topic, boolean* value) {
  setSubscribe(topic, value, 0, "");
}

boolean Frog::setSubscribe(char* topic, boolean* value, byte QoS) {
  setSubscribe(topic, value, QoS, "");
}

boolean Frog::setSubscribe(char* topic, boolean* value, char* boolType) {
  setSubscribe(topic, value, 0, boolType);
}
 
boolean Frog::setSubscribe(char* topic, boolean* value, byte QoS, char* boolType) {
  if ((DB_nextID<=__frog_max_topics) & (strlen(topic) <= __frog_max_topic_length)){
    DB_mqtt[DB_nextID].topic = topic;
    DB_mqtt[DB_nextID].boolValue = value;
    DB_mqtt[DB_nextID].flags = ((QoS<<1)& __DB_flags_QoS) | __DB_flags_SUB;
    if (boolType == "ON/OFF") DB_mqtt[DB_nextID].flags |= __DB_flags_TXT;
    DB_nextID++;
    return true;
  } else {
    return false;
  }
}

boolean Frog::setSubscribe(char* topic, float* value) {
  setSubscribe(topic, value, 0);
}

boolean Frog::setSubscribe(char* topic, float* value, byte QoS) {
  if ((DB_nextID<=__frog_max_topics) & (strlen(topic) <= __frog_max_topic_length)){
    DB_mqtt[DB_nextID].topic = topic;
    DB_mqtt[DB_nextID].floatValue = value;
    DB_mqtt[DB_nextID].flags = (((QoS<<1) & __DB_flags_QoS) | __DB_flags_FLT | __DB_flags_SUB);
    DB_nextID++;
    return true;
  } else {
    return false;
  }
}

void Frog::connect() {
  if (!(MQTT_url == NULL)) {
    FrogClient->connect(MQTT_url, MQTT_Port);
  } else {
    FrogClient->connect(MQTT_Broker, MQTT_Port);
  }
  if (FrogClient->connected()) {
    memset(MQTT_stream,0,__frog_max_stream);
    MQTT_streamLength=2;
    if (MQTT_version) {
      streamBuildMessage("MQIsdp");
      MQTT_stream[MQTT_streamLength++] = 3;
    } else {
      streamBuildMessage("MQTT");
      MQTT_stream[MQTT_streamLength++] = 4;
    }
    MQTT_stream[MQTT_streamLength] = __CON_flags_ClSe;
    if (MQTT_user != NULL) MQTT_stream[MQTT_streamLength] = MQTT_stream[MQTT_streamLength] | __CON_flags_auth;
    MQTT_streamLength++;
    MQTT_stream[MQTT_streamLength++] = ((__MQTT_keepAlive & 0xff00) >> 8);
    MQTT_stream[MQTT_streamLength++] = (__MQTT_keepAlive & 0x00ff);
    streamBuildMessage(MQTT_ClientID);        
    if (MQTT_user != NULL) {
      streamBuildMessage(MQTT_user);
      streamBuildMessage(MQTT_password);
    }
    streamBuildFixedHeader(__CONNECT, 0);
  }
  for (ID=0; ID <DB_nextID; ID++) {
    DB_mqtt[ID].flags = DB_mqtt[ID].flags & (__DB_flags_ACK ^ 0xff);
  }
  MQTT_alive=true;
}

void Frog::connack() {
  if (MQTT_stream[MQTT_streamPos+3] == 0) {
    MQTT_alive=true;
  } else if (MQTT_stream[MQTT_streamPos+3] == 1) {
    MQTT_version = !MQTT_version;
    MQTT_alive=false;
  }
}

void Frog::publishClient() {
  char *identifier;
  byte typeFlags;
  memset(MQTT_stream,0,__frog_max_stream);
  MQTT_streamLength=2; 
  streamBuildMessage(DB_mqtt[ID].topic);
  typeFlags = DB_mqtt[ID].flags & __DB_flags_QoS;
  if (!((DB_mqtt[ID].flags & __DB_flags_QoS) == 0)) {
    MQTT_stream[MQTT_streamLength++] = ((ID>>8) & 0x00ff);
    MQTT_stream[MQTT_streamLength++] = (ID & 0x00ff);
  }
  if ((DB_mqtt[ID].flags & __DB_flags_FLT) == 0) {
    if (*DB_mqtt[ID].boolValue) {
      identifier = DB_boolTypeTRUE(ID);
    } else {
      identifier = DB_boolTypeFALSE(ID);
    }
    DB_mqtt[ID].boolLastValue = *DB_mqtt[ID].boolValue;
  } else {
    identifier = floatString;
    String(*DB_mqtt[ID].floatValue).toCharArray(identifier, String(*DB_mqtt[ID].floatValue).length() +1);
    DB_mqtt[ID].floatLastValue = *DB_mqtt[ID].floatValue;
  }
  int streamPos;
  while (!identifier[streamPos]==NULL) {
    MQTT_stream[MQTT_streamLength++] = identifier[streamPos++];
  }
  streamBuildFixedHeader(__PUBLISH, typeFlags);
  if ((DB_mqtt[ID].flags & __DB_flags_QoS) == 0) {
    DB_mqtt[ID].flags = (DB_mqtt[ID].flags & (__DB_flags_ACK ^ 0xff)) | (__DB_flags_ACK & 0xc0);
  } else {
    DB_mqtt[ID].flags = (DB_mqtt[ID].flags & (__DB_flags_ACK ^ 0xff) | (__DB_flags_ACK & 0x40));
  }
}

void Frog::publishServer() {
  int lenTopic = (MQTT_stream[MQTT_streamPos + 2] <<8) + MQTT_stream[MQTT_streamPos + 3];
  int lenPacket = (MQTT_stream[MQTT_streamPos + 0] <<8) + MQTT_stream[MQTT_streamPos + 1];
  for (ID=0; ID < DB_nextID; ID++) {
    int n = 0;
    while ((char(MQTT_stream[MQTT_streamPos + 4 + n]) == *(DB_mqtt[ID].topic + n)) & (n <= (lenTopic+1))) {
      n++;   
    }
    if (n == (lenTopic)) break;
  }
  if (ID < DB_nextID) {
    memset(floatString, 0, 12);
    if (((MQTT_stream[MQTT_streamPos] & __DB_flags_QoS)>>1) == 0) {
      int n = 0;
      while (((lenTopic + n) < lenPacket) & (n <= 12)) {
        floatString[n] = char(MQTT_stream[MQTT_streamPos + 4 + lenTopic + n]);
        n++;
      }
    } else {
      int n = 0;
      while (((lenTopic + n) < (lenPacket)) & (n <= 12)) {
        floatString[n] = char(MQTT_stream[MQTT_streamPos + 6 + lenTopic + n]);
        n++;
      }
    }
    if ((DB_mqtt[ID].flags & __DB_flags_FLT) == 0) {
      //Boolean 
     if (String(floatString) == String(DB_boolTypeTRUE(ID))) {
        *DB_mqtt[ID].boolValue = true;
     } else if (String(floatString) == String(DB_boolTypeFALSE(ID))) {
        *DB_mqtt[ID].boolValue = false;
      }
    } else {
      *DB_mqtt[ID].floatValue = String(floatString).toFloat();
    }
    if (((MQTT_stream[MQTT_streamPos] & __DB_flags_QoS)>>1) == 1) {
      MQTT_stream[MQTT_streamAppendLength++] = __PUBACK | 0x00;
      MQTT_stream[MQTT_streamAppendLength++] = 0x02;
      MQTT_stream[MQTT_streamAppendLength++] = MQTT_stream[MQTT_streamPos + lenTopic + 4];
      MQTT_stream[MQTT_streamAppendLength++] = MQTT_stream[MQTT_streamPos + lenTopic + 5];
    } else if (((MQTT_stream[MQTT_streamPos] & __DB_flags_QoS)>>1) == 2) {
      MQTT_stream[MQTT_streamAppendLength++] = __PUPREC | 0x00;
      MQTT_stream[MQTT_streamAppendLength++] = 0x02;
      MQTT_stream[MQTT_streamAppendLength++] = MQTT_stream[MQTT_streamPos + lenTopic + 4];
      MQTT_stream[MQTT_streamAppendLength++] = MQTT_stream[MQTT_streamPos + lenTopic + 5];
    }
  }
}

void Frog::pubrelClient() {
  MQTT_stream[MQTT_streamAppendLength++] = __PUBREL | 0x02;
  MQTT_stream[MQTT_streamAppendLength++] = 0x02;
  MQTT_stream[MQTT_streamAppendLength++] = MQTT_stream[MQTT_streamPos +2];
  MQTT_stream[MQTT_streamAppendLength++] = MQTT_stream[MQTT_streamPos +3];
}

void Frog::pubcompClient() {
  MQTT_stream[MQTT_streamAppendLength++] = __PUBCOMP | 0x00;
  MQTT_stream[MQTT_streamAppendLength++] = 0x02;
  MQTT_stream[MQTT_streamAppendLength++] = MQTT_stream[MQTT_streamPos +2];
  MQTT_stream[MQTT_streamAppendLength++] = MQTT_stream[MQTT_streamPos +3];
}

void Frog::pubackServer() {
  streamReadACK(3);
}

void Frog::pubcompServer() {
  streamReadACK(3);
}

void Frog::subscribeClient() {
  memset(MQTT_stream,0,__frog_max_stream);
  MQTT_streamLength=2; 
  MQTT_stream[MQTT_streamLength++] = ((ID>>8) & 0x00ff);
  MQTT_stream[MQTT_streamLength++] = (ID & 0x00ff);
  streamBuildMessage(DB_mqtt[ID].topic);
  MQTT_stream[MQTT_streamLength++] = (DB_mqtt[ID].flags & __DB_flags_QoS)>>1;
  streamBuildFixedHeader(__SUBSCRIBE, 0x02);
  if ((DB_mqtt[ID].flags & __DB_flags_ACK) == 0) {
    DB_mqtt[ID].flags = (DB_mqtt[ID].flags & (__DB_flags_ACK ^ 0xff)) | (__DB_flags_ACK & 0xc0);
  } else {
    DB_mqtt[ID].flags = (DB_mqtt[ID].flags & (__DB_flags_ACK ^ 0xff)) | (__DB_flags_ACK & 0x40);
  }
}

void Frog::subackServer() {
  streamReadACK(3);
}

void Frog::pingrespServer() {
  lastResponse = millis() + __MQTT_nextPing;
 }

void Frog::pingreqClient() {
  MQTT_streamLength = 2;
  streamBuildFixedHeader(__PINGREQ, 0);
 }

char* Frog::DB_boolTypeTRUE(int PackID) {
  if ((DB_mqtt[PackID].flags & __DB_flags_TXT) == 0) {
    return boolValTRUE;
  } else {
    return boolCharTRUE;
  }
}

char* Frog::DB_boolTypeFALSE(int PackID) {
  if ((DB_mqtt[PackID].flags & __DB_flags_TXT) == 0) {
    return boolValFALSE;
  } else {
    return boolCharFALSE;
  }
}

int Frog::streamReadACK(int value) {
  ID = ((MQTT_stream[MQTT_streamPos +2]<<8) + MQTT_stream[MQTT_streamPos +3]);
  DB_mqtt[ID].flags = (DB_mqtt[ID].flags & (__DB_flags_ACK ^ 0xff)) | (__DB_flags_ACK & (value<<6));
  return ID;
}

String Frog::streamReadString(int stringPos, int stringEnd) {
  String ret;
  for ( stringPos; stringPos < stringEnd; stringPos++) {
    ret = ret + String(MQTT_stream[stringPos]);
  }
  return ret;
}

void Frog::streamBuildFixedHeader(int ControlPacketType, int TypeFlag) {
  MQTT_stream[0] = (byte(TypeFlag) & 0x0f) | (byte(ControlPacketType) & 0xf0);
  MQTT_stream[1] = (MQTT_streamLength - 2);
  FrogClient->write(MQTT_stream, MQTT_streamLength);
}

void Frog::streamBuildMessage(char* streamArray) {
  int arrayLength=0;
  while (!streamArray[arrayLength]==NULL) {
    MQTT_stream[MQTT_streamLength + arrayLength + 2] = streamArray[arrayLength];
    arrayLength++;
  }
  MQTT_stream[MQTT_streamLength] = (arrayLength >> 8);
  MQTT_stream[MQTT_streamLength + 1] = (arrayLength & 0x00FF);
  MQTT_streamLength = MQTT_streamLength + arrayLength + 2;
}

