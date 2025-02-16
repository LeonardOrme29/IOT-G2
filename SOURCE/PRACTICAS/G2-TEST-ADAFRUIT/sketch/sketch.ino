#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT22.h>

//====LED=======
#include <Wire.h>
#include <hd44780.h>                      // Biblioteca principal
#include <hd44780ioClass/hd44780_I2Cexp.h> // Clase para comunicación I2C

//=================================================================
// Configuración de red WiFi
#define WLAN_SSID "FAMOV"
#define WLAN_PASS "17200214"

// Configuración de Adafruit IO (MQTT)
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "LeonardOrmeno29"
#define AIO_KEY         "aio_mqbS93hQeD3RkJv0HTc5wsYQDZsl"

//=================================================================
// pin
#define SENSOR_PIN 4    
#define PIN_DC     21   
#define MOTOR_PIN  18   

//CREAR OBJETOS
DHT22 dht22(SENSOR_PIN);

hd44780_I2Cexp lcd;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// FEEDS
Adafruit_MQTT_Publish temperatureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidityFeed    = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");

//=================================================================
// WIFI
void connectWiFi() {
  Serial.print("Conectando a WiFi: ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi conectado.");
}

//=================================================================
// CONECTAR A Adafruit IO
void connectMQTT() {
  int8_t ret;
  if (mqtt.connected()) return;
  
  Serial.println("Conectando a MQTT...");
  while ((ret = mqtt.connect()) != 0) {
    Serial.print("Error al conectar a MQTT: ");
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Reintentando en 5 segundos...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT conectado.");
}

//=================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\nInicializando sistema...");

  connectWiFi();

  // Configurar pines de salida
  pinMode(PIN_DC, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);

  //----------INICIO DE LCD----------------
  // Iniciar la pantalla LCD de 16x2
  lcd.begin(16, 2);
  // Encender luz de fondo
  lcd.backlight();
  // Mostrar "Hola Mundo!" en la primera línea
  lcd.setCursor(0, 0);
  lcd.print("Hola Mundo!");
  // Mostrar "desde ESP32" en la segunda línea
  lcd.setCursor(0, 1);
  lcd.print("lEONARDO ORMENO");
}

//=================================================================

void loop() {
  connectMQTT();

  float temperatura = dht22.getTemperature();
  float humedad     = dht22.getHumidity();


  if (dht22.getLastError() != dht22.OK) {
    Serial.print("Error al leer el sensor: ");
    Serial.println(dht22.getLastError());
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temperatura, 1);
    Serial.println(" °C");
    Serial.print("Humedad: ");
    Serial.print(humedad, 1);
    Serial.println(" %");

    // Publicar datos
    if (!temperatureFeed.publish(temperatura)) {
      Serial.println("Error al enviar la temperatura.");
    } else {
      Serial.println("Temperatura enviada exitosamente.");
    }
    if (!humidityFeed.publish(humedad)) {
      Serial.println("Error al enviar la humedad.");
    } else {
      Serial.println("Humedad enviada exitosamente.");
    }
  }

  // Si la temperatura es mayor a 30°C, enciende LED
  if (temperatura > 30) {
    digitalWrite(PIN_DC, HIGH);
    Serial.println("Activado: PIN_DC (temperatura alta).");
  } else {
    digitalWrite(PIN_DC, LOW);
  }
  
  // Si la temperatura es mayor a 40°C, enciende el motor
  if (temperatura > 40) {
    digitalWrite(MOTOR_PIN, HIGH);
    Serial.println("Activado: MOTOR_PIN (temperatura muy alta).");
  } else {
    digitalWrite(MOTOR_PIN, LOW);
  }

  mqtt.processPackets(10000);

  delay(2000);
}
