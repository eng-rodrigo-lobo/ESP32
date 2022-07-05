/*
  DHT22 + Modbus TCP/IP
  O programa faz a leitura de temperatura e umidade através do sensor DHT22 e transmite os dados para um mestre TCP/IP
  Placa utilizada: ESP32
  Modbus master: ScadaBR
  Display dos dados lidos pela ESP32 será feito via ScadaBR

  ***** Especificações Arduino IDE para compilar ***** 
 * ESP32 Dev Module, upload speed 921600, freq. CPU 240 MHz (Wifi/BT),
 * Freq. FLASH 80 MHz, FLASH mode QIO, FLASH Size 4MB, Partition Scheme: default 4MB with spiffs
 * Core debug level: nenhum, PSRAM: disabled
*/

//BIBLIOTECAS
#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>
#include "DHT.h"

//----------- Variáveis e construtores ----------------
//Entrar com o nome e a senha da sua rede WiFi 
const char* ssid = "";
const char* password = "";

//Variável para atualização de leitura usando millis()
long ts;
//Intervalo de envio das mensagens
int intervalo = 2000;


// Pino D4 conectado ao sensor DHT22
#define DHTPIN 4
// Modelo do sensor: DHT 22
#define DHTTYPE DHT22

//Construtor que inicializa a leitura do sensor
DHT dht(DHTPIN, DHTTYPE);

//Modbus Registers Offsets
const int IREG_TEMP = 100;
const int IREG_UMID = 101;

//ModbusIP object
ModbusIP mb;
//-----------------------------------------------------

void setup()
{
  Serial.begin(9600);
  setup_wifi();
  Serial.println(F("DHT22 - TEMPERATURA E UMIDADE"));
  dht.begin();              // Inicia a leitura do sensor DHT22
  mb.server();		          // Inicia o Modbus IP

  // Adicionando 2 registradores do tipo INPUT, código modbus 0x03, read-only para guardar variáveis do tipo int de até 16-bits
  // É necessário dividir por 10 os valores salvos nos registradores para ter a leitura correta
  mb.addIreg(IREG_TEMP);    // Adiciona o registrador IREG_TEMP = 100 para registrar os valores de temperatura
  mb.addIreg(IREG_UMID);    // Adiciona o registrador IREG_UMID = 101 para registrar os valores de umidade
  
  ts = millis();            // Inicia o marcador de tempo para leituras
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


void loop() {
   //Call once inside loop() - all magic here
   mb.task();

   //Leitura a cada 2 segundos
   if (millis() > ts + intervalo)
   {
    ts = millis();
    //Registra os valores int das funções temp() e umid() nos endereços 100 e 101
    mb.Ireg(IREG_TEMP, temp()); mb.Ireg(IREG_UMID, umid());
   }
   delay(10);
}


int temp()
{
  float t = dht.readTemperature();
  int t_out = int(t*10);
  Serial.print(F("Temperatura: ")); Serial.print(t); Serial.print(F("°C - Input reg 0x30100: "));Serial.println(t_out);
  return t_out;
}

int umid()
{
  float h = dht.readHumidity();
  int h_out = int(h*10);
  Serial.print(F("Umidade: ")); Serial.print(h); Serial.print(F("% - Input reg 0x30101: "));Serial.println(h_out);
  return h_out;
}
