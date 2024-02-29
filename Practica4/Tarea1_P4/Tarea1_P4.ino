#include <ArduinoBLE.h>

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";

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
