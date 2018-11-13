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

#define RELE D1
#define BUTTON  D2
#define RECONFIGURAPIN D5
void interrupts_();
//Parametros da conexão
WiFiClient espClient;

//Parametros do device
SaIoTDeviceLib fechadura("ECT-repro", "PRT120418192750", "ricardo@email.com");
SaIoTController onOff("{\"key\":\"ON\",\"class\":\"button\",\"tag\":\"ON\"}");
String senha = "12345678910";
volatile bool abrindo = false;
void blinkyButton();
void interrupts_();
//Variveis controladores
volatile bool reconfigura = false;
volatile long lastTime;
volatile unsigned long previousMillis = 0;
volatile long interval = 4000;
volatile unsigned long lastreply = 0;
volatile unsigned long lastsend = 0;
volatile unsigned long lastbutton = 0;



//Funções controladores
void setReconfigura();
void setOn(String);
void blinky(int port);
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
  pinMode(RECONFIGURAPIN, INPUT_PULLUP);
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
  if (!abrindo && (json == "1"))
  {
    Serial.println("Abrindo porta...");
    abrindo != abrindo;
    blinky(RELE);
    if(fechadura.reportController(onOff.getKey(), 0)){
      Serial.println("Ok");
    }else{
      Serial.println("Error reportMe");
    }
  }else {
      digitalWrite(RELE, LOW);
  }
}

void blinky(int port){
  for (size_t i = 0; i < 5; i++) {
    digitalWrite(port, HIGH);
    delay(50);
    digitalWrite(port, LOW);
    delay(50);
  }
}
void blinkyButton(){
  if ((millis() - lastbutton) >= 200) {
    Serial.println("botão");
    blinky(RELE);
    Serial.flush();
    lastbutton = millis();
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
  });attachInterrupt(RECONFIGURAPIN, setReconfigura, FALLING);
  attachInterrupt(BUTTON, blinkyButton, FALLING);
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


void interrupts_(){
  attachInterrupt(digitalPinToInterrupt(RECONFIGURAPIN), setReconfigura, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON), blinkyButton, FALLING);
}
//
// void detachInterrupt_(){
//   detachInterrupt(digitalPinToInterrupt(RECONFIGURAPIN));
//   deAttachInterrupt(digitalPinToInterrupt(BUTTON));
// }
