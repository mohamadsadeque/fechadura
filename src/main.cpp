#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SaIoTDeviceLib.h>

#define DEBUG true
#define FECHADURA_PIN D1
#define RECONFIGURAPIN D5

//Parametros da conexão
WiFiClient espClient;

//Parametros do device
SaIoTDeviceLib fechadura("Porta ECT", "PRT120418192750", "ricardo@email.com");
SaIoTController onOff("ON", "ABRIR", "button");
String senha = "12345678910";
bool abrindo = false;

//Funções controladores
void setReconfigura();
void setOn(String);
//Funções conexão
void callback(char *topic, byte *payload, unsigned int length);
//Funções padão
void loop();
void setup();

//Variveis controladores
bool reconfigura = false;

long lastTime;

unsigned long previousMillis = 0;
long interval = 4000;
unsigned long lastreply = 0;
unsigned long lastsend = 0;

void setup()
{

  fechadura.addController(onOff);
  analogWriteFreq(4);
  Serial.begin(115200);
  pinMode(RECONFIGURAPIN, INPUT_PULLUP);
  pinMode(FECHADURA_PIN, OUTPUT);
  attachInterrupt(RECONFIGURAPIN, setReconfigura, FALLING);
  fechadura.preSetCom(espClient, callback);
  fechadura.startDefault(senha);
}

void loop()
{
  fechadura.handleLoop();
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
    analogWrite(FECHADURA_PIN, 200);
    delay(2000);
    analogWrite(FECHADURA_PIN, 0);
    if(fechadura.reportController(onOff.getKey(), 0)){
      Serial.println("Ok");
    }else{
      Serial.println("Error reportMe");
    }
  }
}
