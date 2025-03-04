#include <Wire.h>                // Librería para comunicación I2C
#include <LiquidCrystal_I2C.h>    // Librería para controlar pantallas LCD con interfaz I2C
#include <DHT.h>                  // Librería para interactuar con el sensor de temperatura y humedad DHT22
#include <ESP32Servo.h>           // Librería para controlar servomotores con el ESP32

// Definición de pines
#define LDR_PIN         34    // Sensor de luz (LDR) conectado al pin 34
#define YELLOW_LED_PIN  32    // LED amarillo conectado al pin 32
#define RED_LED_PIN     17    // LED rojo conectado al pin 17
#define SERVO_PIN       23    // Servomotor conectado al pin 23
#define DHT_PIN         4     // Sensor DHT22 conectado al pin 4

// Inicialización de dispositivos
LiquidCrystal_I2C LCD(0x27, 20, 4); // Configuración del LCD con dirección 0x27 y pantalla de 20x4 caracteres
DHT dht(DHT_PIN, DHT22);            // Configuración del sensor DHT22 en el pin 4
Servo servo;                        // Crear un objeto para controlar el servomotor

void setup() {
  Serial.begin(115200);             // Inicializa la comunicación serial con velocidad 115200 baudios
  
  // Configuración de la pantalla LCD
  LCD.init();                       // Inicializa la pantalla LCD
  delay(1000);                      // Espera 1 segundo para asegurar que el LCD esté listo
  LCD.backlight();                  // Enciende la luz de fondo del LCD
  LCD.setCursor(0, 0);              // Establece el cursor en la primera fila y columna del LCD
  LCD.print("Inicializando...");    // Muestra el mensaje "Inicializando..." en el LCD

  // Configuración de pines
  pinMode(RED_LED_PIN, OUTPUT);     // Configura el pin del LED rojo como salida
  pinMode(YELLOW_LED_PIN, OUTPUT);  // Configura el pin del LED amarillo como salida
  
  dht.begin();                      // Inicializa el sensor DHT22
  servo.attach(SERVO_PIN);          // Conecta el servomotor al pin 23
}

void loop() {
  // Lee los valores de humedad y temperatura del sensor DHT22
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Lee el valor del sensor de luz (LDR) y lo invierte
  int ldrValue = analogRead(LDR_PIN);
  int lightLevel = 4095 - ldrValue;  // Invertir la lectura de la LDR, ya que la lectura más baja indica más luz

  // Verifica si los valores de los sensores son válidos (no NaN)
  if (isnan(humidity) || isnan(temperature) || isnan(ldrValue)) {
    delay(2000);  // Si algún valor es inválido, espera 2 segundos y vuelve a leer
    return;
  }

  // Mostrar los valores leídos en la pantalla LCD
  LCD.clear();                     // Borra la pantalla LCD para actualizar los valores
  LCD.setCursor(0, 0);             // Establece el cursor en la primera fila
  LCD.print("Temp: ");
  LCD.print(temperature, 1);       // Muestra la temperatura con 1 decimal
  LCD.print("C");

  LCD.setCursor(0, 1);             // Establece el cursor en la segunda fila
  LCD.print("Humedad: ");
  LCD.print(humidity, 1);          // Muestra la humedad con 1 decimal
  LCD.print("%");

  LCD.setCursor(0, 2);             // Establece el cursor en la tercera fila
  LCD.print("Luz: ");
  LCD.print(lightLevel);           // Muestra el nivel de luz (valor invertido de LDR)

  // Mostrar los valores leídos en el monitor serie
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print("C  Humedad: ");
  Serial.print(humidity);
  Serial.print("%  Luz: ");
  Serial.println(lightLevel);

  // Control del servomotor para ventilación
  if (temperature > 30.0) {
    servo.write(180);  // Abre la ventilación (ángulo máximo del servomotor)
  } else if (temperature < 18.0) {
    servo.write(0);    // Cierra la ventilación (ángulo mínimo del servomotor)
  }

  // Control del LED rojo para alertas ambientales
  if (humidity < 50.0 || humidity > 80.0 || temperature < 18.0 || temperature > 30.0) {
    digitalWrite(RED_LED_PIN, HIGH);  // Enciende el LED rojo si los valores están fuera del rango seguro
  } else {
    digitalWrite(RED_LED_PIN, LOW);   // Apaga el LED rojo si los valores están dentro del rango seguro
  }

  // Control del LED amarillo para la luz ambiente
  if (ldrValue < 2000) {
    digitalWrite(YELLOW_LED_PIN, LOW); // Apaga el LED amarillo si hay poca luz
  } else {
    digitalWrite(YELLOW_LED_PIN, HIGH); // Enciende el LED amarillo si hay suficiente luz
  }

  delay(2000);  // Espera 2 segundos antes de realizar la siguiente lectura
}