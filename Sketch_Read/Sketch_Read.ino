#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
#include <WiFi.h>
#include <PubSubClient.h>

//*********************************************
//****************** PINES  ********************
//*********************************************
const int HX711_dout = 4; //HX711 dout 
const int HX711_sck = 5; //HX711 sck   


//*********************************************
//************** HX711 CONFIG *****************
//*********************************************
HX711_ADC Celula(HX711_dout, HX711_sck);
const int calVal_eepromAdress = 0;
unsigned long t = 0;



//*********************************************
//*************** WIFI CONFIG *****************
//*********************************************
const char* ssid = "CLAROEAB1";
const char* pass = "CARMINA70";


//*********************************************
//*************** MQTT CONFIG *****************
//*********************************************
const char *mqtt_server = "10.0.0.9";
const int mqtt_port = 1883;
const char *mqtt_user = "esp32";
const char *mqtt_pass = "pucmm";
const char *root_topic_subscribe= "yehudy/subscribe";
const char *root_topic_publish = "yehudy/publish";


//*********************************************
//*************** GLOBALES ********************
//*********************************************

WiFiClient espClient;
PubSubClient client(espClient);
char msg[25];
long count=0;

//*********************************************
//************** SETUP WIFI/MQTT **************
//*********************************************
void callback(char* topic, byte* payload, unsigned int length);
void setupWifiMqtt();
void reconnect();

void setupWifiMqtt(){
  delay(100);
  Serial.print("\nConectando a");
  Serial.println(ssid);
  WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print("-");
  }
  Serial.print("\nConectado a");
  Serial.println(ssid);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}


//*********************************************
//************** SETUP BALANZA ****************
//*********************************************
void setup() {
  setupWifiMqtt();
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Iniciando");

  Celula.begin();

  float calibrationValue; // Factor de claibracion
  calibrationValue = 487.01; // setear factor de calibracion
  unsigned long timer = 2000; // timer  para añadir un tiempo de estabilizació; ayuda a una mejor precisión. 
  boolean _tare = false; 
  Celula.start(timer, _tare);
  if (Celula.getTareTimeoutFlag() || Celula.getSignalTimeoutFlag()) {
    Serial.println("Error, debe revisar el circuito");
    while (1);
  }
  else {
    Celula.setCalFactor(calibrationValue); // factor de calibración inicial
    Serial.println("Configuracion completada");
  }

}


//*********************************************
//****************** LOOP ***********************
//*********************************************
void loop() {
  float i = 0;
  //Reconectar 
  unsigned long contador = millis();
  if (!client.connected()){
    reconnect();
  }
  static boolean nextDato = 0;
  unsigned long intervalo = 3000; //incrementar valor para reducir la cantidad de veces que se presenta el peso.
  // revisar si hay nuevos datos
  if (Celula.update()) nextDato = true;
  if (nextDato) {
    if (millis() > t) {
      i = Celula.getData();
      String aux = " gramos";
      Serial.print("Masa: ");
      Serial.println(i + aux);
      nextDato = 0;
      if (client.connected() && (millis() >= t + intervalo)){
        String str = String(i);
        str.toCharArray(msg, 25); 
        client.publish(root_topic_publish, msg);
        t = millis(); 
        }     
    }
   client.loop();
  }

  // recibir comandos de la linea serial
  if (Serial.available() > 0) {
    char dataReceived = Serial.read();
    if (dataReceived == 't') Celula.tareNoDelay(); //iniciar tare
  }

  // revisar si el tare fue completado exitosamente
  if (Celula.getTareStatus() == true) {
    Serial.println("Tare completado");
  }

}


//*****************************
//****** RECONEXION MQTT ******
//*****************************

void reconnect() {

  while (!client.connected()) {
    Serial.print("Intentando conexión Mqtt...");
    // Crear un cliente ID
    String clientId = "IOTICOS_H_W_";
    clientId += String(random(0xffff), HEX);
    // Intentar conectar
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("Conectado!");
      // Suscribir
      if(client.subscribe(root_topic_subscribe)){
        Serial.println("Suscripcion ok");
      }else{
        Serial.println("fallo Suscripciión");
      }
    } else {
      Serial.print("falló :( con error -> ");
      Serial.print(client.state());
      Serial.println(" Intentamos de nuevo en 5 segundos");
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length){
  String incoming = "";
  Serial.print("Mensaje recibido desde -> ");
  Serial.print(topic);
  Serial.println("");
  for (int i = 0; i < length; i++) {
    incoming += (char)payload[i];
  }
  incoming.trim();
  Serial.println("Mensaje -> " + incoming);

}
