/*
  DHT22 + MQTT
  O programa faz a leitura de temperatura e umidade através do sensor DHT22 e transmite para um broker MQTT
  Placa utilizada: ESP32
  Broker MQTT: Mosquitto
  Servidor do Broker: Raspberry Pi 3
  Display dos dados lidos pela ESP32 será feito via Node-RED rodando na RPi3
*/

//BIBLIOTECAS
#include "DHT.h"
#include <WiFi.h>
#include <PubSubClient.h>

//----------- Variáveis e construtores ----------------
//Entrar com o nome e a senha da sua rede WiFi 
const char* ssid = "";
const char* password = "";

//Entrar com o IP fixo do broker MQTT e a porta
const char* mqtt_server = "";
const int mqtt_port = 1883;

//Entrar com usuário e senha do broker MQTT
const char* mqtt_user = "";
const char* mqtt_password = "";

//Definir tópicos a publicar
const char* mqtt_topic_temp = "esp32/temp";
const char* mqtt_topic_umid = "esp32/umid";

//Iniciando construtores WiFI e PubSubClient
WiFiClient espClient;
PubSubClient client(espClient);

//Variável para atualização de leitura usando millis()
long lastMsg = 0;
//Intervalo de envio das mensagens
int intervalo = 10000;

// Pino D4 conectado ao sensor DHT22
#define DHTPIN 4
// Modelo do sensor: DHT 22
#define DHTTYPE DHT22

//Construtor que inicializa a leitura do sensor
DHT dht(DHTPIN, DHTTYPE);
//-----------------------------------------------------


void setup()
{
  Serial.begin(9600);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  Serial.println(F("DHT22 - TEMPERATURA E UMIDADE"));
  dht.begin();
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Conectando a rede wifi: "); Serial.println(ssid);

  WiFi.begin(ssid,password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500); Serial.print(".");  
  }

  Serial.println("");
  Serial.println("Wifi conectado!");
  Serial.print("IP "); Serial.println(WiFi.localIP());
}

float temp()
{
  float t = dht.readTemperature();
  Serial.print(F("Temperatura: ")); Serial.print(t); Serial.println(F("°C "));
  return t;
}

float umid()
{
  float h = dht.readHumidity();
  Serial.print(F("Umidade: ")); Serial.print(h); Serial.println(F("% "));
  return h;
}

void reconectabroker()
{
  //Conexao ao broker MQTT
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected())
  {
    Serial.println("Conectando ao broker MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password ))
    {
      Serial.println("Conectado ao broker!");
    }
    else
    {
      Serial.print("Falha na conexao ao broker - Estado: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}


void loop() 
{
  if (!client.connected()) { reconectabroker(); }

  long now = millis();
  if(now - lastMsg > intervalo)
  {
    lastMsg = now;
    //Envia leitura de umidade para o broker
    char umidString[8];
    dtostrf(umid(), 1, 2, umidString);
    Serial.print("Umidade: "); Serial.println(umidString);
    client.publish(mqtt_topic_umid, umidString);

    //Envia leitura de temperatura para o broker
    char tempString[8];
    dtostrf(temp(), 1, 2, tempString);
    Serial.print("Temperatura: "); Serial.println(tempString);
    client.publish(mqtt_topic_temp, tempString);
  }
    
}
