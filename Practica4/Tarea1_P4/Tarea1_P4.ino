#include <ArduinoBLE.h>

BLEService chatService("19B10000-E8F2-537E-4F6C-D104768A1214"); // UUID del servicio (identificador universal)
//BLEStringCharacteristic nameCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); // UUID del campo característico
BLEStringCharacteristic nameCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 20); // UUID del campo característico
  // 20 = tamaño máximo de la cadena de caracteres, la caracteristica permite operaciones de lectura o escritura

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Inicialización BLE fallida");
    while (1);
  }

  BLE.setLocalName("Marta"); // se establece el nombre del dispositivo

  //nameCharacteristic.setValue("Marta"); // Establecer el valor del campo característico

  BLE.advertise(); // comienza el advertising
  Serial.println("Esperando una conexión BLE...");
}

void loop() {

}
