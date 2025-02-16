#include <WiFi.h>
#include "secret.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT22.h>

//====LED=======
#include <Wire.h>
#include <hd44780.h>                      // Biblioteca principal
#include <hd44780ioClass/hd44780_I2Cexp.h> // Clase para comunicación I2C

#define AWS_IOT_PUBLISH_TOPIC   "iotfrontier/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "iotfrontier/sub"

const int DHT_PIN = 4;
#define PIN_LED     18   
#define MOTOR_PIN  19
const int PHOTOSENSOR_PIN = 32;  // Pin ADC1 seleccionado

//CREAR OBJETO
DHT22 dhtSensor(DHT_PIN);
hd44780_I2Cexp lcd;
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  Serial.println("Connecting to AWS IOT");
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  //client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void publishMessage(float temperatura, float humedad) {
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperatura;
  doc["humidity"] = humedad;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
/*
void automaticActionMotorDC(){
  if(temperatura>=40){
    digitalWrite(pinLED, HIGH); 
    Serial.print("MOTOR DC ENCENDIDO");
  }
  else{
    digitalWrite(pinLED, LOW);}
    Serial.print("MOTOR DC APAGADO");
}

void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("Incoming: ");
  Serial.println(topic);
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}*/

void setup() {
  Serial.begin(115200);
  // Configurar pines de salida
  pinMode(PIN_LED, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);

  //----------INICIO DE LCD----------------
  // Iniciar la pantalla LCD de 16x2
  lcd.begin(16, 2);
  // Encender luz de fondo
  lcd.backlight();
  // Mostrar "Hola Mundo!" en la primera línea
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...!");

  Serial.println("Initializing...");
  connectAWS();
}

void loop() {
  float temperatura = dhtSensor.getTemperature();
  int humedad = dhtSensor.getHumidity();
  int lectura = analogRead(PHOTOSENSOR_PIN);
  int iluminacion = map(lectura, 4095, 0, 0, 100); // Convertir a porcentaje (0-100%)

  Serial.print("Temperature: ");
  Serial.print(temperatura, 2);
  Serial.println("°C");

  Serial.print("Humidity: ");
  Serial.print(humedad, 1);
  Serial.println("%");

  Serial.print("Iluminacion: ");
  Serial.print(iluminacion);
  Serial.println("%");

  if (temperatura > 30) {
    digitalWrite(PIN_LED, HIGH);
    Serial.println("Activado: PIN_LED (temperatura alta).");
  } else {
    digitalWrite(PIN_LED, LOW);
  }

  if (temperatura > 40) {
    digitalWrite(MOTOR_PIN, HIGH);
    Serial.println("Activado: MOTOR_PIN (temperatura muy alta).");
  } else {
    digitalWrite(MOTOR_PIN, LOW);
  }

  // --------- LCD ---------
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperatura, 1);
  lcd.print("C ");

  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(humedad, 1);
  lcd.print("% L:");
  lcd.print(lectura);  // Mostrar iluminación
  lcd.print("%");

  publishMessage(temperatura, humedad);
  client.loop();
  delay(3000);
}