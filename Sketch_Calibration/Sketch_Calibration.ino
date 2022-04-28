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
  unsigned long timer = 2000; // timer  para añadir un tiempo de estabilizació; ayuda a una mejor precisión. 
  boolean _tare = true; 
  Celula.start(timer, _tare);
  if (Celula.getTareTimeoutFlag() || Celula.getSignalTimeoutFlag()) {
    Serial.println("Error, debe revisar el circuito");
    while (1);
  }
  else {
    Celula.setCalFactor(1.0); // factor de calibración inicial
    Serial.println("Configuracion completada");
  }
  while (!Celula.update());
  calibration(); //iniciar proceso de calibración
}


//*********************************************
//****************** LOOP ***********************
//*********************************************
void loop() {
  //Reconectar 
  if (!client.connected()){
    reconnect();
  }
  static boolean nextDato = 0;
  const int intervalo = 0; //incrementar valor para reducir la cantidad de veces que se presenta el peso.
  // revisar si hay nuevos datos
  if (Celula.update()) nextDato = true;

  if (nextDato) {
    if (millis() > t + intervalo) {
      float i = Celula.getData();
      String aux = " gramos";
      Serial.print("Masa: ");
      Serial.println(i + aux);
      nextDato = 0;
      t = millis();
  if (client.connected()){
    String str = String(Celula.getData());
    str.toCharArray(msg, 25); 
    client.publish(root_topic_publish, msg);
    delay(50);
     }      
    }
   client.loop();
  }

  // recibir comandos de la linea serial
  if (Serial.available() > 0) {
    char dataReceived = Serial.read();
    if (dataReceived == 't') Celula.tareNoDelay(); //iniciar tare
    else if (dataReceived == 'r') calibration(); //calibrar el sistema
  }

  // revisar si el tare fue completado exitosamente
  if (Celula.getTareStatus() == true) {
    Serial.println("Tare completado");
  }

}

//*********************************************
//************** CALIBRACION ******************
//*********************************************

void calibration() {
  Serial.println("***");
  Serial.println("Iniciar calibracion:");
  Serial.println("Coloca la balanza en una superficie estable.");
  Serial.println("Remover cualquier peso sobre la balanza.");
  Serial.println("Enviar 't' para iniciar el tare");

  boolean _resume = false;
  while (_resume == false) {
    Celula.update();
    if (Serial.available() > 0) {
      if (Serial.available() > 0) {
        char dataReceived = Serial.read();
        if (dataReceived == 't') Celula.tareNoDelay();
      }
    }
    if (Celula.getTareStatus() == true) {
      Serial.println("Tare completado");
      _resume = true;
    }
  }

  Serial.println("Coloca una masa conocida sobre la balanza");
  Serial.println("Envia el valor de la masa conocida");

  float masaConst = 0;
  _resume = false;
  while (_resume == false) {
    Celula.update();
    if (Serial.available() > 0) {
      masaConst = Serial.parseFloat();
      if (masaConst != 0) {
        Serial.print("Masa conocida: ");
        Serial.println(masaConst);
        _resume = true;
      }
    }
  }

  Celula.refreshDataSet(); //refrescar dataset para verificar valor
  float constCalibration = Celula.getNewCalibration(masaConst); //utilizar nuevo valor de calibracion

  Serial.print("Factor de calibracion: ");
  Serial.print(constCalibration);
  Serial.print("Guardar este valor ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char dataReceived = Serial.read();
      if (dataReceived == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, constCalibration);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, constCalibration);
        Serial.print("Value ");
        Serial.print(constCalibration);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;

      }
      else if (dataReceived == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }

  Serial.println("Calibracion finalizada");
  Serial.println("***");
  Serial.println("Para recalibrar, enviar 'r' desde la linea serial.");
  Serial.println("Para editar el valor manualmente, enviar 'c'");
  Serial.println("***");
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
