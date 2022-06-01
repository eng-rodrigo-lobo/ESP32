/*
  DHT22 + MQTT
  O programa faz a leitura de temperatura e umidade através do sensor DHT22 e transmite para um broker MQTT
 REQUIRES the following Arduino libraries:
 - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
 - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
*/

#include "DHT.h"

#define DHTPIN 4        // Pino D4 conectado ao sensor DHT22
#define DHTTYPE DHT22   // Modelo do sensor: DHT 22

DHT dht(DHTPIN, DHTTYPE);   //Inicializa a leitura do sensor

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHT22 - TEMPERATURA E UMIDADE"));
  dht.begin();
}

void loop() {
  // Aguarda 2 seg entre cada medição
  delay(2000);

  // Leituar
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Para ler a temperatura em Fahrenheit basta colocar o argumento 'true' na função readTemperature (isFahrenheit = true)
  //float f = dht.readTemperature(true);

  // Verifica se houve erro na leitura
  if (isnan(h) || isnan(t)) { Serial.println(F("*** Falha na leitura do sensor! ***")); return; }

  // Calcular o índice de calor em Fahrenheit (padrão)
  // float hif = dht.computeHeatIndex(f, h);
  
  // Calcular o índice de calor em Celsius (isFahrenheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Umidade: ")); Serial.print(h); Serial.print(F("%  Temperatura: ")); Serial.print(t); Serial.print(F("°C ")); Serial.print(F(" Indice de calor: ")); Serial.print(hic); Serial.println(F("°C "));
}
