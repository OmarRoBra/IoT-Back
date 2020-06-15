# Control de un invernadero

## Contenido:

- Archivo .ino 
- Aplicación para el manejo de naves: `https://epic-yonath-50f60d.netlify.app/`
- Repositorio del código de frontend: `https://github.com/OmarRoBra/IoT-App`
- Librería Firebase_ESP8266_Client 
- Link para descargar la librería Sparkfun BMP180: `https://www.luisllamas.es/medir-presion-del-aire-y-altitud-con-arduino-y-barometro-bmp180/`

#### Modificaciones necesarias para el funcionamiento correcto del código en cualquier placa ESP8266:

1. Incluir las librería Firebase_ESP8266_Client y sparkfun bmp180 como .zip en Arduino IDE
2. Desde el administrador de librerías/bibliotecas de Arduino IDE, añadir las siguientes: 

    - Servo by Michael Margolis
    - DHT sensor library by Adafruit
3. Configuraciones necesarias:  para la conexión a internet en: WIFI_SSID y WIFI_PASSWORD
Para agregar nave: incluir el nombre de la nave  asignada en el APP CRUD 
*String path = "/naves/NOMBREMINAVE";*
4. Sensores incluidos con sus respectivos pines conectados a la placa:

    - Servomotor - 2 (**D4**)
    - LDR - 12 (**D6**)
    - DHT11 - 14 (**D5**)
    - BMP180: 

        - SCL - (**D1**)
        - SDA - (**D2**)
    - Ultrasónico:

        - trigpin - 15 (**D8**)
        - echopin - 13 (**D7**)
5. Relays (LEDS) conectados:

    - Foco (LDR) - 16 (**D0**)
    - Bomba (servomotor) - 0 (**D3**)
6. LEDS conectados para alertas:

    - Alerta de humedad - 1 (**tx**)
    - Alerta de presión - 3 (**rx**)
    - Alerta de distancia - led interno
    - Alerta de temperatura - bomba 

7. Sensor de ultrasonido recibe 5V

