#include <Wire.h>               
#include <LiquidCrystal_I2C.h>   
#include <DHT.h>                
#include <ESP32Servo.h>         
#include <WiFi.h>
#include <ThingSpeak.h>

const char* WIFI_NAME = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const int myChannelNumber = 2828545;
const char* myApiKey = "UOI3ZDWZWOCL60FH";
const char* server = "api.thingspeak.com";

// Definición de pines
#define LDR_PIN         34    // Sensor de luz (LDR)
#define YELLOW_LED_PIN  32    // LED amarillo
#define RED_LED_PIN     17    // LED rojo
#define SERVO_PIN       23    // Servomotor
#define DHT_PIN         4     // Sensor DHT22

// Inicialización de dispositivos
LiquidCrystal_I2C LCD(0x27, 20, 4);
DHT dht(DHT_PIN, DHT22);
Servo servo;  

WiFiClient  client;

void setup() {
  Serial.begin(115200);
  
  // Configuración de la pantalla LCD
  LCD.init();
  delay(1000);
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("Inicializando...");

  // Configuración de pines
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  
  dht.begin();
  servo.attach(SERVO_PIN); // Conectar servomotor

  // Conexión a WiFi
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");

  // Conexión a ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int ldrValue = analogRead(LDR_PIN);
  int lightLevel = 4095 - ldrValue;  // Invertir la lectura

  if (isnan(humidity) || isnan(temperature) || isnan(ldrValue)) {
    delay(2000);
    return;
  }

  // Mostrar valores en la pantalla LCD
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Temp: ");
  LCD.print(temperature, 1);
  LCD.print("C");

  LCD.setCursor(0, 1);
  LCD.print("Humedad: ");
  LCD.print(humidity, 1);
  LCD.print("%");

  LCD.setCursor(0, 2);
  LCD.print("Luz: ");
  LCD.print(lightLevel);

  // Mostrar en el monitor serie
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print("C  Humedad: ");
  Serial.print(humidity);
  Serial.print("%  Luz: ");
  Serial.println(lightLevel);

  // Enviar los datos a ThingSpeak
  ThingSpeak.setField(1, temperature);  // Field1 = Temp
  ThingSpeak.setField(2, humidity);     // Field2 = Humedad
  
  // Enviar los datos a ThingSpeak cada 20 segundos
  int httpCode = ThingSpeak.writeFields(myChannelNumber, myApiKey);
  if(httpCode == 200) {
    Serial.println("Datos enviados correctamente a ThingSpeak");
  } else {
    Serial.println("Error al enviar datos a ThingSpeak");
  }

  // Control del servomotor para ventilación
  if (temperature > 30.0) {
    servo.write(180);  // Abre ventilación
  } else if (temperature < 18.0) {
    servo.write(0);    // Cierra ventilación
  }

  // Control de LED rojo para alertas ambientales
  if (humidity < 50.0 || humidity > 80.0 || temperature < 18.0 || temperature > 30.0) {
    digitalWrite(RED_LED_PIN, HIGH);  // Enciende LED rojo si hay valores fuera del rango
  } else {
    digitalWrite(RED_LED_PIN, LOW);
  }

  // Control de iluminación (LED amarillo)
  if (ldrValue < 2000) {
    digitalWrite(YELLOW_LED_PIN, LOW); // Enciende LED si hay poca luz
  } else {
    digitalWrite(YELLOW_LED_PIN, HIGH);
  }

  delay(2000); // Esperar antes de la siguiente lectura
}
