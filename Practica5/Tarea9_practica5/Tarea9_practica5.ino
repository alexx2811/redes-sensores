#include <FTPClient_Generic.h>
#include <FTPClient_Generic.hpp>
#include <TimeLib.h>
#include <WiFi.h>
#include <WiFiClient.h>


const char* ssid = "Marta"; // Nombre de tu red WiFi
const char* password = "Marta2001"; // Contraseña de tu red WiFi

const char* ftpServer = "127.0.0.1"; // Dirección IP del servidor FTP
const uint16_t ftpPort = 14148; // Puerto del servidor FTP
const char* ftpUser = "rsense"; // Nombre de usuario del servidor FTP
const char* ftpPassword = "marta"; // Contraseña del servidor FTP

const unsigned long interval = 10000; // Intervalo de tiempo entre cada lectura en milisegundos (10 segundos)
unsigned long previousMillis = 0;

WiFiClient ftpClient;

void setup() {
  Serial.begin(115200);
  delay(10);

  // Conexión a WiFi
  Serial.println();
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a la red WiFi");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    float temperature = obtenerTemperatura();

    guardarJSON(temperature);
  }
}

float obtenerTemperatura() {
  // Generación de temperatura aleatoria entre 20°C y 30°C
  return random(2000, 3000) / 100.0;
}

void guardarJSON(float temperatura) {
  // Crear el nombre del archivo basado en la fecha y hora actual
  char filename[20];
  sprintf(filename, "grupo01_%02d%02d%02d.json", day(), month(), year() % 100);

  // Abrir conexión FTP
  if (ftpClient.connect(ftpServer, ftpPort)) {
    Serial.println("Conectado al servidor FTP");
    
    // Iniciar sesión en el servidor FTP
    if (ftpClient.login(ftpUser, ftpPassword)) {
      Serial.println("Inicio de sesión exitoso");

      // Abrir el archivo en modo escritura binaria
      if (ftpClient.put(filename)) {
        Serial.println("Archivo enviado correctamente");

        // Generar el contenido JSON
        String output = generarJSON(temperatura);

        // Enviar el contenido JSON al servidor FTP
        ftpClient.print(output);
        ftpClient.stop();

        Serial.println("Datos enviados al servidor FTP");
      } else {
        Serial.println("Error al abrir el archivo en el servidor FTP");
      }
    } else {
      Serial.println("Inicio de sesión fallido");
    }
  } else {
    Serial.println("Conexión al servidor FTP fallida");
  }
}

String generarJSON(float temperatura) {
  // Crear el documento JSON
  DynamicJsonDocument doc(256); // Tamaño del documento JSON

  doc["bt"] = time(nullptr); // Marca de tiempo actual
  JsonArray entries = doc.createNestedArray("e");
  JsonObject entry = entries.createNestedObject();
  entry["n"] = "temperature";
  entry["u"] = "Cel";
  entry["v"] = temperatura;

  // Convertir el documento JSON a una cadena
  String output;
  serializeJson(doc, output);

  return output;
}
