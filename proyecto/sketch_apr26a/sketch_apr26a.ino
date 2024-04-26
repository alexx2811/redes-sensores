#include <Arduino_LSM9DS1.h>
#include <timer.h>
#include <timerManager.h>
#include "timer.h"

#define VECTOR_SIZE 3000 
float vec_GyroY[VECTOR_SIZE], vec_copiaGyroY[VECTOR_SIZE];
float x_giro, y_giro, z_giro;
int cont_vecGyroY=0, pos_subida20, pos_bajada, pos_final_subida, Tmuestreo = 5;
bool flagTimerSample = false;
float rango_detect = 20.0, rango_detect_0 = 0.0;
Timer timer;

// Definir los estados posibles de la máquina de estados
enum Estado {
  REPOSO, SUBIENDO, MANTENER, BAJANDO1, BAJANDO2, PROCESADO
};

Estado estadoActual = REPOSO; // Estado inicial de la máquina de estados

void SampleCallback() {
  flagTimerSample = true;
}


void setup() {
  // Inicializar el puerto serie para la comunicación
  Serial.begin(9600);

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

    if (IMU.gyroscopeAvailable()) { // unidades grados por segundo
      IMU.readGyroscope(x_giro, y_giro, z_giro);
    }

    vec_GyroY[cont_vecGyroY]= z_giro; // la Ygyro del pie es la Zgyro del arduino
    cont_vecGyroY = cont_vecGyroY + 1;
    flagTimerSample = false;
  }

  // Ejecutar el código correspondiente al estado actual
  switch (estadoActual) {
    
    case REPOSO:

      if (vec_GyroY[cont_vecGyroY] < -rango_detect){
        pos_subida20 = cont_vecGyroY;
        estadoActual = SUBIENDO; 
      }
      break;
      
    case SUBIENDO:
      if ((vec_GyroY[cont_vecGyroY] > -rango_detect_0 ) && (vec_GyroY[cont_vecGyroY] < rango_detect_0)){
        pos_final_subida = cont_vecGyroY;
        estadoActual = MANTENER;
      } 
      break;
    
    case MANTENER:
      if ((vec_GyroY[cont_vecGyroY] > -rango_detect_0 ) && (vec_GyroY[cont_vecGyroY] < rango_detect_0)){
        pos_final_subida = cont_vecGyroY;
        estadoActual = BAJANDO1;
      } 
      break;

    case BAJANDO1: // estado auxiliar para asegurarnos de que pasamos un umbral (20 por ej) antes de detectar el 0 (final de movimiento)
      if (vec_GyroY[cont_vecGyroY] > rango_detect){
        estadoActual = BAJANDO2;
      }
      break;

    case BAJANDO2:
      if ((vec_GyroY[cont_vecGyroY] > -rango_detect_0 ) && (vec_GyroY[cont_vecGyroY] < rango_detect_0)){
        pos_bajada = cont_vecGyroY;
        for (int i = 0; i < VECTOR_SIZE; i++) {
          vec_copiaGyroY[i] = vec_GyroY[i];
        } 
        cont_vecGyroY = 0;
        estadoActual = PROCESADO;
      }
      break; 
     
    case PROCESADO:
      
      int i = pos_subida20;
      float acc = 0.0, angulo_maximo, angulo20;
      int tiempo20, tsubida, tbajada, tTotal;
      
      // integracion y tiempo de la subida
      while (vec_copiaGyroY[i] < -rango_detect_0 ){
        acc = acc + vec_copiaGyroY[i];
        i = i - 1;  
      }
      tiempo20 = (pos_subida20 - i) * Tmuestreo;
      angulo20 = acc * Tmuestreo;
      acc = 0.0;

      for (i = pos_subida20; i < pos_final_subida ; i++) {
        acc = acc + vec_copiaGyroY[i];
      }
      tsubida = tiempo20 + (pos_final_subida - pos_subida20 ) * Tmuestreo; // Tiempo que tarda en ponerse de puntillas
      angulo_maximo = angulo20 + acc * Tmuestreo; // angulo maximo
      break; 

      // integracion y tiempo de la bajada
      tbajada = (pos_bajada - pos_final_subida ) * Tmuestreo;
      break;

      //RESULTADOS
      tTotal = tsubida + tbajada; // tiempo que permanece de puntillas
      estadoActual = REPOSO;
  }
  
}
