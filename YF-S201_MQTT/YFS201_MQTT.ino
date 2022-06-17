/*
  YF-S201 + MQTT
  O programa faz a leitura do fluxo de um fluido (água ou ar) do sensor YF-S201 e transmite para um broker MQTT
  Placa utilizada: ESP8266 (NodeMCU)
  Broker MQTT: Mosquitto
  Servidor do Broker: Raspberry Pi 3
  Display dos dados lidos pela NodeMCU será feito via Node-RED rodando na RPi3
*/

//BIBLIOTECAS
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
const char* mqtt_topic_fluxo    = "nodeMCU/fluxo";
const char* mqtt_topic_L        = "nodeMCU/volume_L";
const char* mqtt_topic_m3       = "nodeMCU/volume_m3";

//Iniciando construtores WiFI e PubSubClient
WiFiClient nodeMCU_Client;
PubSubClient client(nodeMCU_Client);

//Variável para atualização de leitura usando millis()
long lastMsg = 0;
//Intervalo de envio das mensagens em milissegundos
int intervalo = 1000; //a cada 1 seg

// pino de entrada do sensor
#define YFS201 2

// variáveis de interrupção
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;

// variáveis de saída
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}


//--------------------------------------------------------
//------------------------ SETUP -------------------------
//--------------------------------------------------------
void setup()
{
  Serial.begin(9600);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  Serial.println(F("YF-S201 - VAZAO E VOLUME DE AGUA"));
  pinMode(YFS201, INPUT_PULLUP);
 
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  attachInterrupt(digitalPinToInterrupt(YFS201), pulseCounter, FALLING);
}

//--------------------------------------------------------
//------------------------ MAIN LOOP ---------------------
//--------------------------------------------------------
void loop() 
{
  if (!client.connected()) { reconectabroker(); }

  long now = millis();
  if(now - lastMsg > intervalo)
  {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    
    //Por conta deste loop talvez não ser executado em exatamente intervalos de 1 segundo, calculamos o número de milisegs que passaram desde a última execução e usamos esse número para calcular a escala da saída. Aplicamos também o fator de calibração para escalonar a saída de acordo com o número de pulsos por segundo por unidade de medida (litros/min nesse caso) que é fornecido pelo sensor.
    flowRate = ((1000.0 / (millis() - lastMsg)) * pulse1Sec) / calibrationFactor;
    lastMsg = millis();

    // Dividimos o valor do fluxo [L/min] por 60 para determinar quantos litros passaram pelo sensor no intervalo de 1 segundo, então multiplicamos por 1000 para converter para mililitros
    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);
 
    //Somamos os mL que passaram nesse segundo para a variável cumulativa total
    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;
    
    //Imprime os dados de leitura na porta serial
    Serial.print("Fluxo: ");
    Serial.print(float(flowRate));  // Print the integer part of the variable
    Serial.print(" L/min");

    Serial.print("   -   Volume: ");
    Serial.print(float(totalLitres));  // Print the integer part of the variable
    Serial.println(" L");
    /* 
    const char* mqtt_topic_fluxo    = "nodeMCU/fluxo";
    const char* mqtt_topic_mL       = "nodeMCU/volume_mL";
    const char* mqtt_topic_L        = "nodeMCU/volume_L";
    */

    //-------------- Envio dos dados para tópicos no broker MQTT -------------- 
    //fluxo
    char fluxo_Str[8];
    dtostrf(flowRate, 1, 2, fluxo_Str);
    client.publish(mqtt_topic_fluxo, fluxo_Str);

    //volume em litros
    char volume_L_Str[8];
    dtostrf(totalLitres, 1, 2, volume_L_Str);
    client.publish(mqtt_topic_L, volume_L_Str);
  }
    
}

//--------------------------------------------------------
//------------------------ FUNÇÕES -----------------------
//--------------------------------------------------------

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

void reconectabroker()
{
  //Conexao ao broker MQTT
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected())
  {
    Serial.println("Conectando ao broker MQTT...");
    if (client.connect("nodeMCU_Client", mqtt_user, mqtt_password ))
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
