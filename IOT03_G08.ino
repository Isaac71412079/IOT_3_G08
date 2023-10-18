#include <WiFi.h>
#include <PubSubClient.h>
const char * WIFI_SSID = "TECHLAB";
const char * WIFI_PASS = "catolica11";

const char * MQTT_BROKER = "broker.hivemq.com";
const int MQTT_BROKER_PORT = 1883;

const char * MQTT_CLIENT_ID = "ESP-1";
const char * MQTT_USR_NAME = "ESP32";
const char * SUBSCRIBE_TOPIC = "Demo";
const char * PUBLISH_TOPIC = "Demo";

// Definición de los pines para el sensor ultrasónico y los LEDs
const int Trigger = 13;       // GPIO 13 para el Trigger del sensor ultrasónico
const int Echo = 12;          // GPIO 12 para el Echo del sensor ultrasónico
const int numLeds = 3;        // Número total de LEDs
int leds[numLeds] = {4, 5, 18};  // Pines GPIO de los LEDs

WiFiClient mqttclient;
PubSubClient mqttClient(mqttclient);

void callback(const char * topic, byte * payload, unsigned int lenght) {
  String message;
  for (int i = 0; i < lenght; i++) {
    message += String((char) payload[i]);
  }
  if (message == "turnonred"){
    digitalWrite(4, HIGH);
  }
  else
  {
    if(message == "turnoffred"){
      digitalWrite(4, LOW);
    }
  }
  if (message == "turnonyellow"){
    digitalWrite(5, HIGH);
  }
  else{
    if(message == "turnoffyellow"){
    digitalWrite(5, LOW);
    }
  }
  if (message == "turnongreen"){
    digitalWrite(18, HIGH);
  }
  else{
    if(message == "turnoffgreen"){
    digitalWrite(18, LOW);
    }
  }
  if (String(topic) == SUBSCRIBE_TOPIC) {
    Serial.println("Message from topic " + String(SUBSCRIBE_TOPIC) + ":" +  message);
  }
}

void reConnect() {
  while(!mqttClient.connected()){
    Serial.print("Connecting to " + String(MQTT_BROKER));
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println(" DONE!");

    mqttClient.subscribe(SUBSCRIBE_TOPIC);
    Serial.println("Subscribed to " + String(SUBSCRIBE_TOPIC));
  } else {
    Serial.println("Can't connect to " + String(MQTT_BROKER));
    Serial.print(mqttClient.state());
    Serial.println(" Reintentando en 5 segundos...");
    delay(5000);
  }
 }
}


void setup() {
  Serial.begin(115200);
  pinMode(Trigger, OUTPUT);  // Configuración del pin Trigger como salida
  pinMode(Echo, INPUT);      // Configuración del pin Echo como entrada
  
  // Configuración de los pines GPIO de los LEDs como salidas
  for (int i = 0; i < numLeds; i++) {
    pinMode(leds[i], OUTPUT);
  } 
  digitalWrite(Trigger, LOW);  // Inicialización del pin Trigger en estado bajo
  
  Serial.print("Connecting to: ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  delay(2000);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" DONE!");

  mqttClient.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  mqttClient.setCallback(callback);
}

unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;

unsigned char counter = 0;

void loop() {

  long t;  // Variable para el tiempo de eco
  long d;  // Variable para la distancia medida
  digitalWrite(Trigger, HIGH);         // Generar pulso de activación del sensor
  delayMicroseconds(10);               // Esperar un corto tiempo
  digitalWrite(Trigger, LOW);          // Finalizar el pulso
  t = pulseIn(Echo, HIGH);             // Medir la duración del pulso de eco
  d = t / 59;                          // Convertir el tiempo en distancia en cm
  Serial.print("Distancia: ");
  Serial.print(d);
  Serial.print("cm");
  Serial.println();
  
  // Encender el LED correspondiente a la distancia medida

  delay(100);  // Esperar un breve periodo de tiempo antes de la siguiente medición
  String message = String(d);
  unsigned long now = millis();
  if (!mqttClient.connected()) {
    reConnect();
    delay(1000);
  } else { // Connected to the MQTT Broker
    mqttClient.loop();
    delay(20);
    
    if (now - previousPublishMillis >= 10000) {
      previousPublishMillis = now;
      // publish
      mqttClient.publish(PUBLISH_TOPIC, message.c_str());
    }
  }
}
