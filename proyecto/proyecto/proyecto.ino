#include <Arduino_LSM9DS1.h>
#include <timer.h>
#include <timerManager.h>
#include "timer.h"
#include <ArduinoBLE.h>

//Declaracion del servicio BLE y sus caracteristicas
BLEService parkinsonService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic tsubida_Characteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify,512);
BLECharacteristic tTotal_Characteristic("19B10001-E8F2-537E-4F6C-D104768A1215", BLERead | BLENotify,512);
BLECharacteristic angulomax_Characteristic("19B10001-E8F2-537E-4F6C-D104768A1216", BLERead | BLENotify,512);
BLECharacteristic angulomin_Characteristic("19B10001-E8F2-537E-4F6C-D104768A1217", BLERead | BLENotify,512);
BLECharacteristic angulomedio_Characteristic("19B10001-E8F2-537E-4F6C-D104768A1218", BLERead | BLENotify,512);
BLEByteCharacteristic resetCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLEWrite);

#define tamanoVector 5000
float vec_GyroY[tamanoVector], vec_copiaGyroY[tamanoVector]; //vectores en los que se almacenan los valores de aceleracion
float x_giro, y_giro, z_giro;
int cont_vecGyroY = 0, pos_subida20, pos_bajada20, pos_final_bajada, pos_final_subida, pos_mantener, Tmuestreo = 20;
float rango_detect = 50.0, rango_detect_0 = 0.2; //rangos para detectivar el movimiento y la parada
bool flagTimerSample = false; //flag interrupcion
bool resetEnabled = true; //para resetear una medida y comenzar otra
Timer timer;

void SampleCallback() {
  flagTimerSample = true;
}

// Definir los estados posibles de la máquina de estados
enum Estado {
  REPOSO,
  SUBIENDO,
  MANTENER,
  BAJANDO,
  PROCESADO
};

Estado estadoActual = REPOSO;  // Estado inicial de la máquina de estados

void setup() {
  
  Serial.begin(9600); // Inicializar el puerto serie para la comunicación
  while (!Serial)
    ;
  Serial.println("Started");

  for (int i = 0; i < tamanoVector; i++) { // Inicializar el vector de aceleraciones en Y a 0.
    vec_GyroY[i] = 0;
  }

  if (!IMU.begin()) { // Inicializar IMU
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }

  if (!BLE.begin()) { // Inicializar Bluetooth
    Serial.println("Failed to initialize BLE!");
    while (1)
      ;
  }

  BLE.setLocalName("Arduino Nano 33 BLE"); 
  BLE.setAdvertisedService(parkinsonService);
  parkinsonService.addCharacteristic(tsubida_Characteristic);
  parkinsonService.addCharacteristic(tTotal_Characteristic);
  parkinsonService.addCharacteristic(angulomax_Characteristic);
  parkinsonService.addCharacteristic(angulomin_Characteristic);
  parkinsonService.addCharacteristic(angulomedio_Characteristic);
  parkinsonService.addCharacteristic(resetCharacteristic);
  BLE.addService(parkinsonService);

  tsubida_Characteristic.setValue(0);  //valores iniciales de las caraceristicas
  tTotal_Characteristic.setValue(0);
  angulomax_Characteristic.setValue(0);
  angulomin_Characteristic.setValue(0);
  angulomedio_Characteristic.setValue(0);
  resetCharacteristic.setValue(0);

  BLE.advertise();  //  El servicio BLE se anuncia para poder conectarse

  Serial.println("BLE Accelerometer Peripheral");


  // los datos de los sensores se muestrean cada Tmuestreo ms
  timer.setInterval(Tmuestreo);
  timer.setCallback(SampleCallback);
  timer.start();
}

void loop() {

  BLEDevice central = BLE.central();  // dispositivo central, manejo de la conexion

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      if (resetCharacteristic.written()) { 
        // numero entero sin signo de 8 bits (almacena valores entre el 0 y 255)
        uint8_t value;
        resetCharacteristic.readValue(value);

        if (value == 1) {
          resetEnabled = true;
          Serial.println("Accelerometer enabled"); //se activa el acelerometro y se procede a medir el movimiento de puntillas
        }

        resetCharacteristic.setValue(0);
      }

      if (resetEnabled) { //se ejecuta cuando activamos la opcion en NRFConnect
        
        if (flagTimerSample) {             
          //Interrupcion
          if (IMU.gyroscopeAvailable()) {  // unidades grados por segundo
            IMU.readGyroscope(x_giro, y_giro, z_giro);
          }
          cont_vecGyroY = cont_vecGyroY + 1;
          vec_GyroY[cont_vecGyroY] = z_giro;  // la Ygyro del pie es la Zgyro del arduino

          flagTimerSample = false;
        }

        // Ejecuta el código correspondiente al estado actual
        switch (estadoActual) {

          case REPOSO:
            if (vec_GyroY[cont_vecGyroY] < -rango_detect) {
              pos_subida20 = cont_vecGyroY;
              Serial.print("contador_subida: ");
              Serial.println(pos_subida20);
              Serial.println("SUBIENDO");
              estadoActual = SUBIENDO;
            }
            break;

          case SUBIENDO:
            if ((vec_GyroY[cont_vecGyroY] > -rango_detect_0) && (vec_GyroY[cont_vecGyroY] < rango_detect_0)) {
              pos_final_subida = cont_vecGyroY;
              Serial.print("final subida contador: ");
              Serial.println(pos_final_subida);
              Serial.println("MANTENER");
              estadoActual = MANTENER;
            }
            break;

          case MANTENER:
            if (vec_GyroY[cont_vecGyroY] > rango_detect) {
              pos_bajada20 = cont_vecGyroY;
              Serial.print("contador_bajada 20: ");
              Serial.println(pos_bajada20);
              Serial.println("BAJANDO");
              estadoActual = BAJANDO;
            }
            break;

          case BAJANDO:
            if ((vec_GyroY[cont_vecGyroY] > -rango_detect_0) && (vec_GyroY[cont_vecGyroY] < rango_detect_0)) {
              pos_final_bajada = cont_vecGyroY;
              Serial.print("contador_final: ");
              Serial.println(pos_final_bajada);
              for (int i = 0; i < tamanoVector; i++) {
                vec_copiaGyroY[i] = vec_GyroY[i];
              }

              cont_vecGyroY = 0;
              Serial.println("PROCESADO");
              estadoActual = PROCESADO;
            }
            break;

          case PROCESADO:

            int i = pos_subida20;
            float acc = 0.0, angulo_final_subida, angulo20subida, angulo_medio, angulo_min_final, angulo_max_final, angulo_max = 0.0, angulo_min = 0.0;
            float tiempo20_subida, tiempo20_bajada, tmantener, tsubida, tTotal;

            // integracion y tiempo de la subida
            while (vec_copiaGyroY[i] < -rango_detect_0) {
              acc = acc + vec_copiaGyroY[i];
              i = i - 1;
            }

            tiempo20_subida = (pos_subida20 - i) * float(Tmuestreo) / 1000;
            angulo20subida = acc * float(Tmuestreo) / 1000;
            acc = 0.0;

            for (i = pos_subida20; i < pos_final_subida; i++) {
              acc = acc + vec_copiaGyroY[i];
            }

            tsubida = tiempo20_subida + (pos_final_subida - pos_subida20) * float(Tmuestreo) / 1000;  // Tiempo que tarda en ponerse de puntillas
            angulo_final_subida = angulo20subida + acc * float(Tmuestreo) / 1000;                     // angulo maximo
            Serial.print("angulo_final_subida: ");
            Serial.println(angulo_final_subida, 2);

            // integracion y tiempo de la bajada
            acc = 0.0;
            int j = pos_bajada20;

            while (vec_copiaGyroY[j] > -rango_detect_0) {
              j = j - 1;
            }
            pos_mantener = j;
            Serial.print("pos_mantener: ");
            Serial.println(pos_mantener);

            float angulos_mantener[pos_mantener - pos_final_subida];

            j = 0;
            for (i = pos_final_subida; i < pos_mantener; i++) {
              acc = acc + vec_copiaGyroY[i];
              angulos_mantener[j] = acc * float(Tmuestreo) / 1000;
              j = j + 1;
            }
            angulo_medio = angulo_final_subida + (acc * float(Tmuestreo) / 1000) / (pos_mantener - pos_final_subida);

            for (i = 0; i < (pos_mantener - pos_final_subida); i++) {
              if (angulos_mantener[i] > angulo_max)
                angulo_max = angulos_mantener[i];
              if (angulos_mantener[i] < angulo_min)
                angulo_min = angulos_mantener[i];
            }
            angulo_min_final = angulo_max + angulo_final_subida;
            angulo_max_final = angulo_min + angulo_final_subida;


            //RESULTADOS
            tmantener = (pos_mantener - pos_final_subida) * float(Tmuestreo) / 1000;
            tTotal = tsubida + (pos_final_bajada - pos_final_subida) * float(Tmuestreo) / 1000;  // tiempo que permanece de puntillas

            Serial.print("tiempo que tarda en ponerse de puntillas: ");
            Serial.println(tsubida, 2);
            String tsubidaStr = String (tsubida);
            tsubida_Characteristic.writeValue(tsubidaStr.c_str());

            Serial.print("tiempo mantener arriba: ");
            Serial.println(tmantener, 2);

            Serial.print("tiempo que tarda en hacer el movimiento: ");
            Serial.println(tTotal, 2);
            String tTotalStr = String (tTotal);
            tTotal_Characteristic.writeValue(tTotalStr.c_str());

            Serial.print("angulo maximo: ");
            Serial.println(angulo_max_final, 2);
            String angulo_max_finalStr = String (angulo_max_final);
            angulomax_Characteristic.writeValue(angulo_max_finalStr.c_str());

            Serial.print("angulo minimo: ");
            Serial.println(angulo_min_final, 2);
            String angulo_min_finalStr = String (angulo_min_final);
            angulomin_Characteristic.writeValue(angulo_min_finalStr.c_str());

            Serial.print("angulo medio: ");
            Serial.println(angulo_medio, 2);
            String angulo_medioStr = String (angulo_medio);
            angulomedio_Characteristic.writeValue(angulo_medioStr.c_str());

            Serial.println("REPOSO");
            estadoActual = REPOSO;
            resetEnabled = false; //desactiva el proceso para tener que activarlo cada vez que se usa
            break;
        }
        timer.update();
        
      }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}
