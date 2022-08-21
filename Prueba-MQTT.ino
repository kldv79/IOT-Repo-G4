//================== SENSOR DHT22 ==============
//Librerías del sensor DHT22 (sensor de humedad y temperatura)
#include <DHT.h>
#include <DHT_U.h>
//====================== PLACA ESP8266 ===========================
//Librerías para la conexión del WIFI
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// ====================== SENSOR DHT22 =========================0
DHT dht (D1, DHT22);
// temp tendra la tempreatura en celcius, temp_f tendra la temperatura en farenheit,
// humed tendra la humedad y hi tendra la sensacion termica.
float temp, temp_f, humed, hi;
//============================ PLACA ESP8266 =======================
//Conexión a la red WIFI
const char* ssid = "bacco 2.4 GHZ"; //nombre de la red
const char* password = "frecuencia34"; //contraseña de la red
const char* mqtt_server = "192.168.0.24"; //ip del broker
//Inicializamos el objeto de cliente esp
WiFiClient espClient;
//Iniciamos el objeto subscriptor del cliente con el objeto del cliente
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (300)
char msg[MSG_BUFFER_SIZE];
int value = 0;
//======================== PLACA ESP8266 ============================0
void setup_wifi() {
   delay(10);
   // Empezamos por conectarnos a una red WiFi
   Serial.println();
   Serial.print("Connecting to "); //Se imprime un mensaje de conectando
   Serial.println(ssid); //Nos muestra el nombre de la red conectada
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   //Proceso q nos menciona q el WIFI está conectado
   randomSeed(micros());
   Serial.println("");
   Serial.println("WiFi connected");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
}
//================= PLACA ESP8266 =================
void callback(char* topic, byte* payload, unsigned int length) {
   Serial.print("Message arrived [");
   Serial.print(topic);
   Serial.print("] ");
   for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
   }
   Serial.println();
   if ((char)payload[0] == '1') {
      digitalWrite(BUILTIN_LED, LOW);
   } else {
      digitalWrite(BUILTIN_LED, HIGH);
   }
}
//================= PLACA ESP8266 =================
void reconnect() {
 while (!client.connected()) {
   Serial.print("Attempting MQTT connection...");
   // Se crea un cliente por default
   //creamos una nueva cadena de conexión para el servidor
   String clientId = "ESP8266Client-";
   clientId += String(random(0xffff), HEX);
   // Intento de conexión
   if (client.connect(clientId.c_str())) {
     Serial.println("connected");
     // Una vez conectado, publica un anuncio...
     client.publish("outTopic", "hello world");
     // ... y volver a suscribirse
     client.subscribe("inTopic");
   } else {
     Serial.print("failed, rc=");
     Serial.print(client.state());
     Serial.println(" try again in 5 seconds");
     // Esperar 5 segundos antes de reintentar
     delay(5000);
   }
 }
}
void setup() {
   //================= PLACA ESP8266 =================
   pinMode(BUILTIN_LED, OUTPUT); //Inicializar el pin BUILTIN_LED como salida
   Serial.begin(115200);
   setup_wifi();
   client.setServer(mqtt_server, 1883); //Numero del puerto
    client.setCallback(callback);
  
   //========================= SENSOR DHT22 ==========================
   dht.begin(); //Funcion usada para inicializar el sensor
}
void loop() {
   //============================= SENSOR DHT22  =============================
   humed = dht.readHumidity(); //Captando la humedad del sensor
   temp = dht.readTemperature (); //Captando la temperatura en grados celcius
   temp_f= dht.readTemperature (true); //Captando la temperatura en grados Farenheit
   hi= dht.computeHeatIndex (temp, humed); // Captando la sensacion termica
   //=========================== SENSOR HUMEDAD DE SUELO ==================
   int humedad_lectura = analogRead(A0); // Variable que capta el valor de la humedad del suelo
   //Variable que tendra el valor de la lectua en forma de procentaje con la funcion map
   int lecturaPorc = map(humedad_lectura, 1023, 0, 0, 100);
   //============================ PLACA ESP8266 ==========================0
   //Preguntamos por el estado de la conexión
   if (!client.connected()) {
    reconnect();
   }
   client.loop();
   unsigned long now = millis();
   if (now - lastMsg > 2000) {
   lastMsg = now;
   ++value;    
     //========================== SENSOR DHT22 =======================
     //Mostrando las lecturas de la la humedad y temperatura en grados celcius y farenheit
     snprintf (msg, MSG_BUFFER_SIZE, "------------SENSOR DE HUMEDAD Y TEMPERATURA - DHT22 ------",value);
     client.publish("TeamG4:", msg);
     // Mostrando la temperatura en grados celcius y la humedad
     snprintf (msg, MSG_BUFFER_SIZE, "Temperature: %1d en Centigrados || Humedad: %1d", char(temp), char(humed));
     client.publish("TeamG4:", msg);
     // Mostrando la temperatura en grados Farenheit
     snprintf (msg, MSG_BUFFER_SIZE, "Temperature: %1d en Fahrenheit", char(temp_f));
     client.publish("TeamG4:", msg);
     // Mostrando la sensacion termica
     snprintf (msg, MSG_BUFFER_SIZE, "Sensacion Termica: %1d \n",char(hi));
     client.publish("TeamG4:", msg);
     //=========================== SENSOR HUMEDAD DEL SUELO ==============
     //La lectura es entre 0 mínimo y 1023 como máximo
     snprintf (msg, MSG_BUFFER_SIZE, "------------SENSOR DE HUMEDAD DEL SUELO ----------",value);
     client.publish("TeamG4:", msg);
     //Mostrando la lectura del sensor de humedad de suelo
     snprintf (msg, MSG_BUFFER_SIZE, "Humedad del suelo: %1d",char(humedad_lectura));
     client.publish("TeamG4:", msg);
     //La lectura en porcentaje
     snprintf (msg, MSG_BUFFER_SIZE, "Humedad del suelo en porcentaje: %1d \n", char(lecturaPorc));
     client.publish("TeamG4:", msg);
     //Mostrando la humedad de suelo en porcentaje
     snprintf (msg, MSG_BUFFER_SIZE,
    "==================================================\n",value);
     client.publish("TeamG4:", msg);
   }
}
