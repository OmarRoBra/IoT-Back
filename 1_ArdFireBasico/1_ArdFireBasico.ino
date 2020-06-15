#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SFE_BMP180.h>

#define FIREBASE_HOST "redes-7f174.firebaseio.com" //Sin http:// o https:// 
#define FIREBASE_AUTH "aC2z4z14uX0ciLoJpbHNBsIsYmlTZUx5rMGKH9kv"
#define WIFI_SSID "INFINITUM7k6f" 
#define WIFI_PASSWORD "408c6f7301"

#define bomba 0
#define SP 2 //pin servo D4
#define D0 16 //Led externo
#define D6 12 //ldr
#define DHTTYPE DHT11 // DHT 11
#define D5 14 //dht11
#define humedadpin 1 //tx 
#define presionpin 3 //rx
Servo servo;
DHT dht(D5,DHTTYPE);
const int trigpin = 15; //D8
const int echopin = 13; //D7
float duracion;
int distancia;
int relay1=0; //LDR - foco
int relay2=0; //Servomotor - bomba

SFE_BMP180 bmp180;
#define PresionNivelMar 1013.25 // presion sobre el nivel del mar en mbar

String path = "/naves/nave3";
//Define un objeto de Firebase
FirebaseData firebaseData;

void printResult(FirebaseData &data);
void CausaError(void);
void InforSetLuzSensor(void);
void InforGetLuzSensor(void);

void setup()
{
  Serial.begin(9600);
  pinMode(humedadpin,OUTPUT);
  pinMode(presionpin,OUTPUT);
  pinMode(D0,OUTPUT);
  pinMode(D5, INPUT); 
  dht.begin();
  pinMode(trigpin,OUTPUT);
  pinMode(echopin,INPUT);
  pinMode(bomba,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  servo.attach(2);  //D4
  servo.write(0);

  if (bmp180.begin())
    Serial.println("BMP180 iniciado");
  else
  {
    Serial.println("Error al iniciar el BMP180");
    while(1);
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a ....");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Conectado con la IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Establezca el tiempo de espera de lectura de la base de datos en 1 minuto (máximo 15 minutos)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  
  //Tamaño y  tiempo de espera de escritura, tiny (1s), small (10s), medium (30s) and large (60s).
  //tiny, small, medium, large and unlimited.
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  //get, getInt, getFloat, getDouble, getBool, getString, getJSON, getArray, getBlob, getFile.
  //set, setInt, setFloat, setDouble, setBool, setString, setJSON, setArray, setBlob and setFile.
}

void loop()
{   
  Serial.println("------------------------------------");
  Serial.println("ACTUALIZAR EL ESTADO DE LOS SENSORES");
  
  //temperatura, presión, altitud 
  char status;
  double temperatura,presion,altitud;
  
  status = bmp180.startTemperature(); //Inicio de lectura de temperatura
  if (status != 0)
  {   
    delay(status); //Pausa para que finalice la lectura
    status = bmp180.getTemperature(temperatura); //Obtener la temperatura
    if (status != 0)
    {
      status = bmp180.startPressure(3); //Inicio lectura de presión
      if (status != 0)
      {        
        delay(status); //Pausa para que finalice la lectura        
        status = bmp180.getPressure(presion,temperatura); //Obtener la presión
        if (status != 0)
        {                  
          Serial.print("Temperatura: ");
          Serial.print(temperatura);
          Serial.print(" *C , ");
          Serial.print("Presion: ");
          Serial.print(presion);
          Serial.print(" mb , ");     
          
          altitud= bmp180.altitude(presion,PresionNivelMar); //Calcular altura
          Serial.print("Altitud: ");
          Serial.print(altitud);
          Serial.println(" m");   
        }      
      }      
    }   
  } 
  
  //distancia
  digitalWrite(echopin, LOW);   // set the echo pin LOW
  digitalWrite(trigpin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigpin,LOW);
  
  duracion = pulseIn(echopin, HIGH);  // measure the echo time (μs)
  distancia = (duracion/2.0)*0.0343;

   if (Firebase.get(firebaseData, path + "/sensores/humedad/nombre" )){
    if(firebaseData.stringData() != NULL){
      //humedad
      float humedad = dht.readHumidity();
      Firebase.get(firebaseData, path + "/sensores/humedad/maximo");
      String hMaximo = firebaseData.stringData();
      Firebase.get(firebaseData, path + "/sensores/humedad/minimo");
      String hMinimo = firebaseData.stringData();
      if(humedad >= hMaximo.toFloat() || humedad <= hMinimo.toFloat()){
        digitalWrite(humedadpin,HIGH);
        delay(200);
        digitalWrite(humedadpin,LOW);
      }
      if (Firebase.setFloat(firebaseData, path + "/sensores/humedad/valor", humedad)){InforSetLuzSensor();}else{CausaError();}
    }
  }else{CausaError(); }
  
  if (Firebase.get(firebaseData, path + "/sensores/temperatura/nombre" )){
    if(firebaseData.stringData() != NULL){
       Firebase.get(firebaseData, path + "/sensores/temperatura/maximo");
      String tMaximo = firebaseData.stringData();
      Firebase.get(firebaseData, path + "/sensores/temperatura/minimo");
      String tMinimo = firebaseData.stringData();
      if(temperatura >= tMaximo.toFloat() || temperatura <= tMinimo.toFloat()){
        digitalWrite(bomba,HIGH);
        delay(200);
        digitalWrite(bomba,LOW);
      }
      if (Firebase.setFloat(firebaseData, path + "/sensores/temperatura/valor", temperatura)){InforSetLuzSensor();}else{CausaError();}
    }
  }else{CausaError(); }
  
  if (Firebase.get(firebaseData, path + "/sensores/distancia/nombre" )){
    if(firebaseData.stringData() != NULL){
      Firebase.get(firebaseData, path + "/sensores/distancia/maximo");
      String dMaximo = firebaseData.stringData();
      Firebase.get(firebaseData, path + "/sensores/distancia/minimo");
      String dMinimo = firebaseData.stringData();
      if(distancia >= dMaximo.toFloat() || distancia <= dMinimo.toFloat()){
          digitalWrite(LED_BUILTIN,HIGH);
          delay(50);
          digitalWrite(LED_BUILTIN,LOW);
      }
      if (Firebase.setFloat(firebaseData, path + "/sensores/distancia/valor", distancia)){InforSetLuzSensor();}else{CausaError();}
    }
  }else{CausaError(); }
  
  if (Firebase.get(firebaseData, path + "/sensores/presion/nombre" )){
    if(firebaseData.stringData() != NULL){
      Firebase.get(firebaseData, path + "/sensores/presion/maximo");
      String pMaximo = firebaseData.stringData();
      Firebase.get(firebaseData, path + "/sensores/presion/minimo");
      String pMinimo = firebaseData.stringData();
      if(presion >= pMaximo.toFloat() || presion <= pMinimo.toFloat()){
        digitalWrite(presionpin,HIGH);
        delay(200);
        digitalWrite(presionpin,LOW);
      }
      if (Firebase.setFloat(firebaseData, path + "/sensores/presion/valor", presion)){InforSetLuzSensor();}else{CausaError();}
    }
  }else{CausaError(); }
  
  if (Firebase.get(firebaseData, path + "/sensores/servomotor/nombre" )){
    if(firebaseData.stringData() != NULL){
      //servomotor
      String estadoBomba;
      Firebase.get(firebaseData, path + "/sensores/servomotor/maximo");
      String sMaximo = firebaseData.stringData();
      Firebase.get(firebaseData, path + "/sensores/servomotor/minimo");
      String sMinimo = firebaseData.stringData();
      if(temperatura >= sMaximo.toFloat()){
        relay2 += 1; 
        digitalWrite(bomba,HIGH);
        servo.write(360);
        estadoBomba = "Abierta";
        delay(1000);
      } else {
        digitalWrite(bomba,LOW);
        servo.write(0); 
        estadoBomba = "Cerrada";
        delay(1000); 
      }
       if (Firebase.setInt(firebaseData, path + "/sensores/servomotor/relay2", relay2)){InforSetLuzSensor();}else{CausaError();}
       if (Firebase.setString(firebaseData, path + "/sensores/servomotor/valor", estadoBomba)){InforSetLuzSensor();}else{CausaError();}
    }
  }else{CausaError(); }
  
  if (Firebase.get(firebaseData, path + "/sensores/altitud/nombre" )){
    if(firebaseData.stringData() != NULL){
         if (Firebase.setFloat(firebaseData, path + "/sensores/altitud/valor", altitud)){InforSetLuzSensor();}else{CausaError();}
     }  
  }else{CausaError(); }
  
  if (Firebase.get(firebaseData, path + "/sensores/luz/nombre" )){
    if(firebaseData.stringData() != NULL){
      //luz
      int valorLDR = digitalRead(D6);
      if(valorLDR==0){
        digitalWrite(D0, LOW);
      } else {
        relay1 += 1;
        digitalWrite(D0, HIGH);
      }
      if (Firebase.setInt(firebaseData, path + "/sensores/luz/valor", valorLDR)){InforSetLuzSensor();}else{CausaError();}  
      if (Firebase.setInt(firebaseData, path + "/sensores/luz/relay1", relay1)){InforSetLuzSensor();}else{CausaError();}  
    }
  }else{CausaError(); }

  Serial.println("------------------------------------");
  Serial.println("   LEER EL ESTADO DE LOS SENSORES   ");
  if (Firebase.getInt(firebaseData, path + "/sensores/luz" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getFloat(firebaseData, path + "/sensores/humedad" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getFloat(firebaseData, path + "/sensores/temperatura" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getFloat(firebaseData, path + "/sensores/distancia" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getFloat(firebaseData, path + "/sensores/presion" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getString(firebaseData, path + "/sensores/servomotor" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getFloat(firebaseData, path + "/sensores/altitud" )){InforGetLuzSensor(); }else{CausaError(); }
  delay(1000);
}

void InforGetLuzSensor(void)
{
  Serial.println("Aprobado");
  Serial.println("Ruta: " + firebaseData.dataPath());
  Serial.println("Tipo: " + firebaseData.dataType());
  Serial.println("ETag: " + firebaseData.ETag());
  Serial.print("Valor: ");
  printResult(firebaseData);
  Serial.println("------------------------------------");
  Serial.println(); 
}

void InforSetLuzSensor(void)
{
  Serial.println("Aprobado");
  Serial.println("Ruta: " + firebaseData.dataPath());
  Serial.println("Tipo: " + firebaseData.dataType());
  Serial.println("ETag: " + firebaseData.ETag());
  Serial.print("Valor: ");
  printResult(firebaseData);
  Serial.println("------------------------------------");
  Serial.println(); 
}

void CausaError(void)
{
  Serial.println("ERROR");
  Serial.println("RAZON: " + firebaseData.errorReason());
  Serial.println("------------------------------------");
  Serial.println();
}

void printResult(FirebaseData &data)
{
    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string")
        Serial.println(data.stringData());
}
