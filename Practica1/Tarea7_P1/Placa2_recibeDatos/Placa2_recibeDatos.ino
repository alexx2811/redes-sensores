#include <Wire.h>

float matriz_datos_Sensores[5][9];
int f, c = 0;
int cont_dato = 0;
bool flagAcabado = false;
void setup() {
  Wire.begin(8);                 // inicializacion de la libreria, placa de lectura dirección = 8 (identificacion del dispositivo por la placa de escritura)
  Wire.onReceive(receiveEvent);  // function that executes whenever data is received from writer
}



void receiveEvent(int howMany) {
  float datoSensor = Wire.read();  // receive a character
  matriz_datos_Sensores[f][c] = datoSensor;
  c = c + 1;
  if (c == 14) {
    f = f + 1;
    c = 0;
    if (f == 3) {
      flagAcabado = true;
      f = 0;
    }
  }
  if (flagAcabado) {
    Serial.println("magnetometro (uT)                        acelerometro                             giroscopo                            ");
    Serial.println("X             Y              Z           X             Y              Z           X             Y              Z       ");
    for (int i = 0; i < 5; i++) {
      // Iterar sobre las columnas
      for (int j = 0; j < 9; j++) {
        // Imprimir el elemento de la matriz
        Serial.print(matriz_datos_Sensores[i][j]);
        Serial.print("\t");  // Agregar un tabulador para separar los elementos
      }
      Serial.println();  // Salto de línea para pasar a la siguiente fila
    }

    flagAcabado = false;
  }
}




void loop() {
  // No es necesario realizar ninguna operacion en el bucle principal
}

