#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <Servo.h>

#define FIREBASE_HOST "redes-7f174.firebaseio.com" //Sin http:// o https:// 
#define FIREBASE_AUTH "aC2z4z14uX0ciLoJpbHNBsIsYmlTZUx5rMGKH9kv"
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define bomba 14 //D5
#define SP 12 //pin servo D6
#define D0 15 //Led externo
#define D6 13 //ldr
Servo servo;
const int trigpin = 2; //D8
const int echopin = 0; //D7
float duracion;
int distancia;
int relay1=0; //LDR - foco
int relay2=0; //Servomotor - bomba
float vref = 3.3;
float resolution = vref/1023;

String path = "/naves/nave5";
//Define un objeto de Firebase
FirebaseData firebaseData;

void printResult(FirebaseData &data);
void CausaError(void);
void InforSetLuzSensor(void);
void InforGetLuzSensor(void);

void setup()
{
  Serial.begin(9600);
  pinMode(D0,OUTPUT);
  pinMode(trigpin,OUTPUT);
  pinMode(echopin,INPUT);
  pinMode(bomba,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  servo.attach(12);  //D4
  servo.write(0);

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
  
 //temperatura
  float temperatura = analogRead(A0);
  temperatura = (temperatura*resolution);
  temperatura = temperatura*100;
  Serial.println(temperatura);  
  
  //distancia
  digitalWrite(echopin, LOW);   // set the echo pin LOW
  digitalWrite(trigpin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigpin,LOW);
  
  duracion = pulseIn(echopin, HIGH);  // measure the echo time (μs)
  distancia = (duracion/2.0)*0.0343;
  
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