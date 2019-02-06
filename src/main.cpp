/*
Projeto para controle da porta de sala
v 1.1
Software:
  Danielly
  Patricio Oliveira
  Ricardo Cavalcanti
Hardware:
  Wesley Wagner
  Mohamad Sadeque
*/

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <SaIoTDeviceLib.h>

bool stateLED = true;
const int LED = 12;
unsigned long int timeLED = 500;
unsigned long int lastTimeLED = 500;

#define RELE 5
#define BUTTON  0
// #define RECONFIGURAPIN 14
//Parametros da conexão
WiFiClient espClient;

//Parametros do device
SaIoTDeviceLib fechadura("portaaaaECT,", "portaaaaECT", "ricardo@email.com");
SaIoTController onOff("{\"key\":\"ON\",\"class\":\"button\",\"tag\":\"ON\"}");
String senha = "12345678910";
volatile bool abrindo = false;
//Variveis controladores
volatile bool reconfigura = false;
volatile long lastTime;
volatile unsigned long previousMillis = 0;
volatile long interval = 4000;
volatile unsigned long lastreply = 0;
volatile unsigned long lastsend = 0;
volatile unsigned long lastbutton = 0;
volatile bool open = false;
unsigned long int lastOpen = 0;
unsigned long int timeRelay = 0;


//Funções controladores
void setReconfigura();
void setOn(String);
void blinky();
//Funções MQTT
void callback(char *topic, byte *payload, unsigned int length);
//Funções padão
void setup();
void loop();
//funções
void setupOTA();

void setup()
{
  Serial.begin(115200);
  // pinMode(RECONFIGURAPIN, INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(RELE, OUTPUT);
  interrupts();
  delay(80);
  fechadura.addController(onOff);
  fechadura.preSetCom(espClient, callback);
  fechadura.startDefault(senha);
  setupOTA();
}

void loop()
{
  
  /*Serial.print("leitura butao: ");
  Serial.println(digitalRead(BUTTON));*/
  int tentativa = 0;
  fechadura.handleLoop();

  if (open) {
    digitalWrite(RELE,HIGH);
    delay(2000);
    digitalWrite(RELE,LOW);
    delay(50);
    Serial.flush();
    open = false;
  }

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(500);
    if (++tentativa>=5) {
      ESP.restart();
    }
  }
  Serial.flush();
}

void callback(char *topic, byte *payload, unsigned int length)
{
  String payloadS;
  Serial.print("Topic: ");
  Serial.println(topic);
  for (unsigned int i = 0; i < length; i++)
  {
    payloadS += (char)payload[i];
  }
  if (strcmp(topic, fechadura.getSerial().c_str()) == 0)
  {
    Serial.println("SerialLog: " + payloadS);
  }
  if (strcmp(topic, (fechadura.getSerial() + onOff.getKey()).c_str()) == 0)
  {
    Serial.println("Value: " + payloadS);
    setOn(payloadS);
  }
}

void setReconfigura()
{
  reconfigura = true;
}

void setOn(String json)
{
  if (json == "1" && (((millis() - lastOpen) >= 2250 )))
  {
    lastOpen = millis();
    open = true;
  }
    }
  



void setupOTA(){

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

/*
void interrupts_(){
   attachInterrupt(digitalPinToInterrupt(RECONFIGURAPIN), setReconfigura, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON), blinkyButton, FALLING);
}

 void detachInterrupt_(){
 detachInterrupt(digitalPinToInterrupt(RECONFIGURAPIN));
   deAttachInterrupt(digitalPinToInterrupt(BUTTON));
}*/