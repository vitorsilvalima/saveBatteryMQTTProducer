/*The MIT License (MIT)
Copyright (c) 2016 Vitor Silva Lima

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the 
Software, and to permit persons to whom the Software is furnished to do so, subject
to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#define SENSORDATA_JSON_SIZE (JSON_OBJECT_SIZE(2))
#define DHTPIN 2 //pino em que o sensor está conectado
const char* ssid = "Vitor SL";
const char* password = "49411658";
const char* mqtt_server = "m10.cloudmqtt.com";
const char* mqtt_user = "ylfqyweh";
const char* mqtt_password = "OmyBRcCKjwpi";
const int mqtt_port = 17892;
const int taxaAtualizacao= 900000; //every 15 minutes!!!
long lastMsg=taxaAtualizacao*-1;//so it can run for the first time 

DHT dht(DHTPIN, DHT22);

WiFiClient espClient;
PubSubClient client(espClient);
const int tamanhoMsg = 50;
char msg[tamanhoMsg];


void setup_wifi() 
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  setup_wifi();
  dht.begin();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}
void toJson(float t, float h, char *json,size_t maxSize)
{
    StaticJsonBuffer<SENSORDATA_JSON_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["temperatura"] = t;
    root["umidade"] = h;
    root.printTo(json, maxSize); 
}
void reconnect() {
  //função para reconectar o módulo ao broker de messageria
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //tentativa de conexão
    if (client.connect("ESP8266Client",mqtt_user,mqtt_password)) {
      Serial.println("connected");
      // mensagem enviado ao broker de messageria quando o módulo se conecta
      client.publish("outTopic", "o módulo está conectado!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      //tempo de espera até tentar novamente
      delay(5000);
    }
  }
}
void loop() {
  long now = millis();
  if(now - lastMsg > taxaAtualizacao){
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    reconnect();
    lastMsg = now;
    float h,t;
    do{
      h=dht.readHumidity(); //ler os dados de humidade do sensor
      t=dht.readTemperature();//ler os dados de temperatura do sensor
    }while(isnan(h) || isnan(t));
    toJson(t,h,msg,tamanhoMsg);//converte os dados para o formato json
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensor/temp", msg);
    client.disconnect();
    WiFi.disconnect(true);
  }
}
