#include <Arduino_LSM9DS1.h>
#include <timer.h>
#include <timerManager.h>
#include "timer.h"

#define VECTOR_SIZE 3000
float vec_GyroY[VECTOR_SIZE], vec_copiaGyroY[VECTOR_SIZE];
float x_giro, y_giro, z_giro;
int cont_vecGyroY = 0, pos_subida20, pos_bajada20, pos_final_bajada, pos_final_subida, pos_mantener, Tmuestreo = 50;
float rango_detect = 20.0, rango_detect_0 = 0.0;
bool flagTimerSample = false;

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
  // Inicializar el puerto serie para la comunicación
  Serial.begin(9600);
    while (!Serial)
    ;
  Serial.println("Started");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }

  // los datos de los sensores se muestrean cada 5 ms
  timer.setInterval(Tmuestreo);
  timer.setCallback(SampleCallback);
  timer.start();
}

void loop() {


  if (flagTimerSample) {  //Interrupcion

    if (IMU.gyroscopeAvailable()) {  // unidades grados por segundo
      IMU.readGyroscope(x_giro, y_giro, z_giro);
    }

    vec_GyroY[cont_vecGyroY] = z_giro;  // la Ygyro del pie es la Zgyro del arduino
    cont_vecGyroY = cont_vecGyroY + 1;

    flagTimerSample = false;
  }

  // Ejecutar el código correspondiente al estado actual
  switch (estadoActual) {

    case REPOSO:
      Serial.println ("estoy en REPOSO"); 
      Serial.println (z_giro,2);
      Serial.print ("contador: "); 
      Serial.println (cont_vecGyroY);

      if (vec_GyroY[cont_vecGyroY] < -rango_detect_0) {
        pos_subida20 = cont_vecGyroY;

        Serial.println ("SUBIENDO");
        estadoActual = SUBIENDO;
      }
      break;

    case SUBIENDO:
      if ((vec_GyroY[cont_vecGyroY] > -rango_detect_0) && (vec_GyroY[cont_vecGyroY] < rango_detect_0)) {
        pos_final_subida = cont_vecGyroY;
        Serial.println ("MANTENER");
        estadoActual = MANTENER;
      }
      break;

    case MANTENER:
      if (vec_GyroY[cont_vecGyroY] > rango_detect) {
        pos_bajada20 = cont_vecGyroY;
        Serial.println ("BAJANDO");
        estadoActual = BAJANDO;
      }
      break;

    case BAJANDO:
      if ((vec_GyroY[cont_vecGyroY] > -rango_detect_0) && (vec_GyroY[cont_vecGyroY] < rango_detect_0)) {
        pos_final_bajada = cont_vecGyroY;
        for (int i = 0; i < VECTOR_SIZE; i++) {
          vec_copiaGyroY[i] = vec_GyroY[i];
        }

        cont_vecGyroY = 0;
        Serial.println ("PROCESADO");
        estadoActual = PROCESADO;
      }
      break;

    case PROCESADO:

      int i = pos_subida20;
      float acc = 0.0, angulo_final_subida, angulo20subida, angulo_medio, angulo_max = 0.0, angulo_min = 0.0;
      int tiempo20_subida, tiempo20_bajada, tmantener, tsubida, tmantenerybajada, tTotal;

      // integracion y tiempo de la subida
      while (vec_copiaGyroY[i] < -rango_detect_0) {
        acc = acc + vec_copiaGyroY[i];
        i = i - 1;
      }
      tiempo20_subida = (pos_subida20 - i) * Tmuestreo;
      angulo20subida = acc * Tmuestreo;
      acc = 0.0;

      for (i = pos_subida20; i < pos_final_subida; i++) {
        acc = acc + vec_copiaGyroY[i];
      }
      tsubida = tiempo20_subida + (pos_final_subida - pos_subida20) * Tmuestreo;  // Tiempo que tarda en ponerse de puntillas
      angulo_final_subida = angulo20subida + acc * Tmuestreo;                     // angulo maximo

      // integracion y tiempo de la bajada
      acc = 0.0;
      int j = pos_bajada20;

      while (vec_copiaGyroY[j] < -rango_detect_0) {
        j = j - 1;
      }
      pos_mantener = j;

      float angulos_mantener[pos_mantener - pos_final_subida];
      
      j = 0;
      for (i = pos_final_subida; i < pos_mantener; i++) {
        acc = acc + vec_copiaGyroY[i];
        angulos_mantener[j] = acc * Tmuestreo;
        j = j + 1;
      }
      angulo_medio = angulo_final_subida + (acc * Tmuestreo) / (pos_mantener - pos_final_subida);

      for (i = 0; i < (pos_mantener - pos_final_subida); i++) {
        if(angulos_mantener[i] > angulo_max)
          angulo_max = angulos_mantener[i];
        if(angulos_mantener[i] < angulo_min)
          angulo_min = angulos_mantener[i];
      }
      angulo_max = angulo_max + angulo_final_subida;
      angulo_min = angulo_min + angulo_final_subida;

      //RESULTADOS
      tmantenerybajada = (pos_final_bajada - pos_final_subida) * Tmuestreo;
      tTotal = tsubida + tmantenerybajada;  // tiempo que permanece de puntillas

      Serial.print ("tiempo que tarda en ponerse de puntillas: ");
      Serial.println (tsubida,2);
      Serial.print ("tiempo que permanece de puntillas: ");
      Serial.println (tTotal,2);
      Serial.print ("angulo maximo: ");
      Serial.println (angulo_max,2);
      Serial.print ("angulo minimo: "); 
      Serial.println (angulo_min,2);
      Serial.print ("angulo medio: "); 
      Serial.println (angulo_medio,2);

      estadoActual = REPOSO;
      break;
  }
  timer.update(); 
}
