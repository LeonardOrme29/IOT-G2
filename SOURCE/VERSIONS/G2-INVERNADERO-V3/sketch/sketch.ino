#include <WiFi.h>
#include "secret.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT22.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>


#define AWS_IOT_PUBLISH_TOPIC   "iotfrontier/pub"

#define AWS_IOT_SUBSCRIBE_TOPIC_MODE "iotfrontier/sub"
#define AWS_IOT_SUBSCRIBE_TOPIC_MOTOR "iotfrontier/sub-motor"
#define AWS_IOT_SUBSCRIBE_TOPIC_LED "iotfrontier/sub-led"
#define AWS_IOT_SUBSCRIBE_TOPIC_CONFIG "iotfrontier/sub-config"

//datos automaticos
int intLed1=30;
int intLed2=35;
int intMotor=35;

//pines esp32
const int DHT_PI N= 4; //sensor de humedad y temp
#define PIN_LED     18   // pin led amarillo
#define MOTOR_PIN   19  //pin del motor
#define PIN_LED2    25  //pin del led rojo
const int PHOTOSENSOR_PIN = 32; //pin del sensor de iluminacion

//declaracion de la librerias con los sensores y actuadores
DHT22 dhtSensor(DHT_PIN);
hd44780_I2Cexp lcd;
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

//decalaracion de los estados
bool ControlMode = false; // Modo AUTOMATICO(false) o MANUAL (true)
bool motorState = false; // Estado del motor
bool ledState = false;

String ledY ="OFF";
String ledR ="OFF";

//funcion para conectar a AWS IOT CORE
void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Conectando a Wi-Fi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConectado a Wi-Fi.");

  // Configurar certificados
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  
  // Configurar AWS IoT
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(messageHandler);
  
  Serial.println("Conectando a AWS IoT...");
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  
  if (!client.connected()) {
    Serial.println("Error de conexión con AWS IoT.");
    return;
  }

  Serial.println("\nConectado a AWS IoT!");
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC_MODE);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC_MOTOR);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC_LED);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC_CONFIG);
  Serial.println("Suscrito al tópico de control.");
}

//FUNCION PARA PUBLICAR DATOS DEL SENSOR
void publishMessage(float temperatura, float humedad, int iluminacion) {
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperatura;
  doc["humidity"] = humedad;
  doc["light"] = iluminacion;
  
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

//FUNCION SUSCRIPTOR
void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en: ");
  Serial.println(topic);
  
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.println("Error al analizar JSON");
    return;
  }
  if (strcmp(topic, AWS_IOT_SUBSCRIBE_TOPIC_MODE) == 0){
    subMode(doc);
    Serial.print("SE ACTIVO O DESACTIVO EL MODO");
  }
  if (strcmp(topic, AWS_IOT_SUBSCRIBE_TOPIC_CONFIG) == 0){
    subConfig(doc);
    Serial.print("configuracion de invernadero");
  }
  if(ControlMode){
    if (strcmp(topic, AWS_IOT_SUBSCRIBE_TOPIC_MOTOR) == 0) {
      subMotor(doc);
      Serial.print("------MANUAL DC--------- ");
    }else if(strcmp(topic, AWS_IOT_SUBSCRIBE_TOPIC_LED) == 0){
      subLed(doc);
      Serial.print("------MANUAL LED--------- ");
    }
  }
}

//METODOS DE LOS SUSCRIPTORES
void subMode(StaticJsonDocument<200> doc){
  if (doc.containsKey("mode")){
    String mode = doc["mode"];
    if(mode == "MANUAL"){
        ControlMode=true;
    }else{
      ControlMode=false;
    }
  }
}

void subMotor(StaticJsonDocument<200> doc){
  if (doc.containsKey("motor")) {
      String command = doc["motor"];
      if (command == "ON") {
          motorState = true;
          Serial.println("Motor encendido desde AWS IoT.");
        } else if (command == "OFF") {
          motorState = false;
          Serial.println("Motor apagado desde AWS IoT.");
        }
    }
}

void subLed(StaticJsonDocument<200> doc) {
  ledY = doc["ledyellow"].as<String>();  // Convertir JSON a String
  ledR = doc["ledred"].as<String>();  
}

void subConfig(StaticJsonDocument<200> doc){
  intLed1=doc["intled1"].as<int>();
  intLed2=doc["intled2"].as<int>();
  intMotor=doc["intmotor"].as<int>();
}


void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  connectAWS();
}

void loop() {
  float temperatura = dhtSensor.getTemperature();
  int humedad = dhtSensor.getHumidity();
  int iluminacion = analogRead(PHOTOSENSOR_PIN);
  Serial.println(intLed1);
  Serial.println(intLed2);
  Serial.println(intMotor);

  //MOSTRAR EN EL LCD 16X2
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temperatura, 1); lcd.print("C H:"); lcd.print(humedad, 1); lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Luz:");
  lcd.print(iluminacion);


  //LOGICA LUCES LED
  if(!ControlMode){
    //CONTROL AUTOMATICO
    if (temperatura >= intLed2) {
      digitalWrite(PIN_LED2, HIGH);
      digitalWrite(PIN_LED, LOW);
      Serial.println("LED ROJO activado (temp alta).");
    } else if (temperatura >= intLed1) {
      digitalWrite(PIN_LED2, LOW);
      digitalWrite(PIN_LED, HIGH);
      Serial.println("LED AMARILLO activado (temp moderada).");
    } else {
      digitalWrite(PIN_LED, LOW);
      digitalWrite(PIN_LED2, LOW);
    }
  }else{
    //CONTROL MANUAL
    if (ledY == "ON") {
    digitalWrite(PIN_LED, HIGH);
    Serial.println("led encendido desde AWS IoT.");
    }else{
      digitalWrite(PIN_LED, LOW);
      Serial.println("led apagado desde AWS IoT.");
    }
    if (ledR == "ON") {
      digitalWrite(PIN_LED2, HIGH);
      Serial.println("led encendido desde AWS IoT.");
    }else{
      digitalWrite(PIN_LED2, LOW);
      Serial.println("led apagado desde AWS IoT.");
    }
  }


  //LOGICA MOTOR
  if(ControlMode){
    //CONTROL MANUAL
    if(motorState){
      digitalWrite(MOTOR_PIN, HIGH);
      Serial.println("MOTOR ENCENDIDO MANUALMENTE");
    }else{
      digitalWrite(MOTOR_PIN, LOW);
      Serial.println("MOTOR APAGADO MANUALMENTE");
    }
  }else{
    //CONTROL AUTOMATICO
    if (temperatura > intMotor) { 
      digitalWrite(MOTOR_PIN, HIGH);
      Serial.println("Motor encendido.");
    }else {
        digitalWrite(MOTOR_PIN, LOW);
        Serial.println("Motor apagado.");
      }
  }

  // Publicar datos en AWS IoT
  publishMessage(temperatura, humedad,iluminacion);

  client.loop();
  delay(3000);
}