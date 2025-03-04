Invernadero
===========

Invernadero Inteligente - G2 - IOT
Este sistema está diseñado para gestionar las condiciones ambientales dentro de un invernadero
utilizando sensores y actuadores. La idea es optimizar las condiciones de cultivo controlando la
temperatura, la humedad y la luz de manera automática, y permitiendo un control manual para ajustar
los dispositivos en tiempo real.

Drive del proyecto: https://drive.google.com/drive/folders/1_I2BVb4j9ld2rngvsprmkwmHdsE5ecMm

Video de demostración: https://drive.google.com/file/d/1Z3IGhfaCXeNE3PbU3qyyT_a2JnxfsXlF/view?usp=drive_link

Integrantes:
- Leonardo Ormeño Vasquez
- David Brian Sandoval Falcon
- Sergio Daniel Quiroz Ardiles
- Tarazona Villar Ivan Hubel
- Montes Perz Josue

![invernadero](https://drive.google.com/uc?id=1DA6OoiwpSnC3P7qN1OYXs95wva98zZkZ)

### Tecnologías

#### ARDUINO IDE
Utilizado para programar y cargar el código en el ESP32, que es el microcontrolador encargado de leer los sensores (temperatura, humedad, luz) y controlar los actuadores.

#### AWS IOT CORE
Permite la comunicación en la nube entre el ESP32 y otros sistemas. Se usa para recibir datos en tiempo real de los sensores y enviar comandos de control a los actuadores a través de MQTT.
![imagen](https://drive.google.com/uc?id=1SpYyssz1yguiKM0ytA-ZHvVdgNcnJVX5)

#### NODE-RED
Plataforma de automatización basada en flujos utilizada para procesar y visualizar los datos del invernadero. Permite crear dashboards personalizados.
![imagen](https://drive.google.com/uc?id=1BKhsvdDO5e-8Tq5gIs9lZ8lNR8LRyRqA)

#### MYSQL
Base de datos utilizada para almacenar registros históricos de temperatura, humedad, luz y tiempo.
![imagen](https://drive.google.com/uc?id=1ygox5nrpOtLIr75IY6Q97MyeskLCNoLQ)


