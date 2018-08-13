#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <SaIoTDeviceLib.h>
#include <PubSubClient.h>
#include<ESP8266HTTPClient.h>
#include<WiFiManager.h>

#define DEBUG true
#define FECHADURA_PIN D1
#define RECONFIGURAPIN D5

//Parametros da conexão
#define HOST "api.saiot.ect.ufrn.br" //"10.7.227.121" //
#define hostHttp "http://api.saiot.ect.ufrn.br/v1/device/auth/login" //"http://10.7.227.121:3001/v1/device/auth/login"
#define PORT  8001 //3003 //MQTT //
#define POSTDISPOSITIVO "/manager/post/device/" // v 1.7


WiFiClient espClient;
PubSubClient mqttClient(espClient);

//Parametros do device
SaIoTDeviceLib fechadura("Porta ECT","PRT120418192750","ricardo@email.com");

SaIoTController onOff("ON","button","ABRIR");
String senha = "12345678910";
bool iniciando = true;
void subscribeCont();
//Funções controladores
void setReconfigura();
void setOn(String);
//Funções conexão
void reconnectMQTT();
void getToken();
void callback(char* topic, byte* payload, unsigned int length);
//Funções padão
void loop();
void setup();

//Variveis controladores
bool reconfigura = false;

long lastTime;

//outras
extern String RID;
extern String Rname;

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

  //init mqttClient
  mqttClient.setServer(HOST, PORT);
  mqttClient.setCallback(callback);

  WiFiManager wifi;
  wifi.autoConnect(fechadura.getSerial().c_str());
  //tokenRecebido = getToken();
  getToken();
  reconnectMQTT();

  if (mqttClient.connected())
  {
    Serial.println("connected");
  }
}

void loop()
{
  mqttClient.loop();
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  iniciando=false;
}

void callback(char* topic, byte* payload, unsigned int length){
  String payloadS;
  Serial.print("Topic: ");
  Serial.println(topic);
  for (unsigned int i=0;i<length;i++) {
    payloadS += (char)payload[i];
  }
  if(strcmp(topic,fechadura.getSerial().c_str()) == 0){
    Serial.println("SerialLog: " + payloadS);
  }
  if(strcmp(topic,(fechadura.getSerial()+onOff.getKey()).c_str()) == 0){
    Serial.println("ABRIR: " + payloadS);
    setOn(payloadS);
  }

}

void setReconfigura(){
  reconfigura = true;
}

void setOn(String json){
  if(!iniciando){
    analogWrite(FECHADURA_PIN, 200);
    delay(2000);
    analogWrite(FECHADURA_PIN, 0);
  }
}

void getToken(){
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status

   HTTPClient http;
   http.begin(hostHttp);  //Specify request destination

   http.addHeader("Content-Type","application/json");

   int httpCode = http.POST("{\"email\":\""+fechadura.getEmail()+"\",\"password\":\""+senha+"\",\"serial\":\"" + fechadura.getSerial() + "\"}");   //Send the request
   fechadura.setToken(http.getString());
   http.end();
   Serial.println(httpCode);
  // for (int i = 0; i < 2; i++){
  //   tokenConnect.remove(tokenConnect.indexOf("\""),1);
  // }
  //Serial.print("retorno: ");
  Serial.println(fechadura.getToken());
   //return tokenConnect;

 }else{
   Serial.println("ERROR GET TOKEN");
   //return "ERROR";

  }
}

void reconnectMQTT() { //pensar em erros! Caso desconectado devido ao token n atualizado, chamar função getToken. Quanto tempo expira?
    while (!mqttClient.connected()) {
      Serial.println("Tentando se conectar ao Broker MQTT" );
      if (mqttClient.connect(fechadura.getSerial().c_str(),fechadura.getEmail().c_str(),(fechadura.getToken().c_str()))) {
        Serial.println("Conectado");
        Serial.println(onOff.getKey());
        subscribeCont();
        String JSON;
        JSON += fechadura.makeJconf();
        mqttClient.publish(POSTDISPOSITIVO,JSON.c_str());
      } else {
        Serial.println("Falha ao Reconectar");
        Serial.println("Tentando se reconectar em 2 segundos");
        delay(2000);
      }
    }
  }

  void subscribeCont(){
    mqttClient.subscribe(fechadura.getSerial().c_str());
    mqttClient.subscribe((fechadura.getSerial()+onOff.getKey()).c_str()); //subscribe da bib n aceita String ???
  }
