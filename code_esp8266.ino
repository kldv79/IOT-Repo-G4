//================== LIBRERIAS ==============
//Librerías del sensor DHT22 (sensor de humedad y temperatura)
#include <DHT.h>
#include <DHT_U.h>

//====================== PLACA ESP8266 ===========================
//Librerías para la conexión del WIFI
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ====================== SENSOR DHT22  =========================0
DHT dht (D6, DHT22);
// temp tendra la tempreatura en celcius, temp_f tendra la temperatura en farenheit, 
// humed tendra la humedad y hi tendra la sensacion termica.
float temp, temp_f, humed, hi; 

//===================== VARIABLES DE RED =======================
//Conexión a la red WIFI
const char* ssid = "bacco 2.4 GHZ";            //nombre de la red
const char* password = "frecuencia34"; //contraseña de la red
const char* mqtt_server = "192.168.0.24";      //ip del broker

//Inicializamos el objeto de cliente esp
WiFiClient espClient;

//Iniciamos el objeto subscriptor del cliente con el objeto del cliente
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (300)
char msg[MSG_BUFFER_SIZE];
int value = 0;


//======================== SETUP DEL WIFI ============================
void setup_wifi() {
  delay(10);
  // Empezamos por conectarnos a una red WiFi
  Serial.println();
  Serial.print("Connecting to "); //Se imprime un mensaje de conectando
  Serial.println(ssid); //Nos muestra el nombre de la red conectada
  //WiFi.mode(WIFI_STA);
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

//==== FUNCION  PARA RECIBIR MENSAJES DE NODE-RED USANDO MQTT ===
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
     messageTemp += (char)payload[i];
  }
  
  if (String(topic) == "esp32/switch") {
    if(messageTemp == "true"){
      Serial.println("on");
      digitalWrite(D8, HIGH);
    }
    else if(messageTemp == "false"){
      Serial.println("off");
      digitalWrite(D8, LOW);
    }
  }
}

//================= RECONEXION =================
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
      client.subscribe("esp32/switch");
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
  pinMode(D8, OUTPUT);
  pinMode(D7, INPUT);
  pinMode(A0,INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883); //Numero del puerto
  client.setCallback(callback);
  //========================= SENSOR DHT22 ==========================
  dht.begin(); //Funcion usada para inicializar el sensor
  delay(500); 
}

void loop() {
  //============================= SENSOR DHT22 =============================
  humed = dht.readHumidity(); //Captando la humedad del sensor
  temp = dht.readTemperature (); //Captando la temperatura en grados celcius
  temp_f= dht.readTemperature (true); //Captando la temperatura en grados Farenheit
  hi= dht.computeHeatIndex (temp, humed); // Captando la sensacion termica

  //=========================== SENSOR HUMEDAD DE SUELO ==================
  int humedad_lectura = analogRead(A0); // Variable que capta el valor de la humedad del suelo
  //Variable que tendra el valor de la lectua en forma de procentaje con la funcion map
  int lecturaSueloPorc = map(humedad_lectura, 0, 1023, 100, 0);

  //============================ PLACA ESP8266 ==========================0
  //Preguntamos por el estado de la conexión 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  delay(10);
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;

    char x[8];
    //Convertimos la variable temperatura a string
    dtostrf(temp,7,3,x);
    //Publicamos el mensaje en el siguiente topico:
    client.publish("esp32/temperature",x);
    //Convertimos la variable temperatura a string
    dtostrf(humed,7,3,x);
    //Publicamos el mensaje en el siguiente topico:
    client.publish("esp32/humed", x);
    //Convertimos la variable temperatura a string
    dtostrf(hi,7,3,x);
    //Publicamos el mensaje en el siguiente topico:
    client.publish("esp32/tempamb", x);
    //Convertimos la variable temperatura a string
    dtostrf(lecturaSueloPorc,7,3,x);
    //Publicamos el mensaje en el siguiente topico:
    client.publish("esp32/porchum", x); 
    
  }
}
