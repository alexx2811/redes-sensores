#include <WiFi.h>
#include <WiFiClient.h>
#include "time.h"

const char* ssid = "Marta";
const char* password = "Marta2001";
const char* host = "172.20.10.3";  // Puedes usar la dirección IP o el nombre de dominio
const int puerto= 4096;
bool EnviarDatos = false;
String dato;
WiFiClient client;




void setup() {
  Serial.begin(9600);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED to WIFI");


  if (client.connect(host, puerto)) {
    Serial.println("Connected to server");
    // Realizar acciones adicionales después de la conexión exitosa
  } else {
    Serial.println("Connection failed");
  }
}




void loop() {

  if (!client.connected()) {
    Serial.println("Connection lost, reconnecting...");

    // Intentar reconectarse al servidor
    if (client.connect(host, puerto)) {
      Serial.println("Reconnected to server");
      // Realizar acciones adicionales después de la reconexión exitosa
    } else {
      Serial.println("Reconnection failed");
      delay(500);
    }
  }
  
  client.print("X_acel = ");
  client.print(random(1,100));
  client.print(", ");
  client.print("Y_acel = ");
  client.print(random(1,100));
  client.print(", ");
  client.print("Z_acel = ");
  client.println(random(1,100));
  delay(100);
}
