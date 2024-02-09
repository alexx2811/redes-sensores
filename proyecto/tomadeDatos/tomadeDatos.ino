#include <Arduino_LSM9DS1.h>
#include <timer.h>
#include <timerManager.h>
#include "timer.h"

float x_acel, y_acel, z_acel;
float x_mag, y_mag, z_mag;
float x_giro, y_giro, z_giro;
int degreesX = 0;
int degreesY = 0;
int degreesZ = 0;
bool flagTimer = false;
Timer timer;

void SampleCallback() {
  flagTimer = true;
}

void setup() {
  
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Started");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }
  // muestreo cada 2 ms
  timer.setInterval(2);
  timer.setCallback(SampleCallback);
  timer.start();
}

void loop() {

  if (flagTimer) {  //Interrupcion

    if (IMU.gyroscopeAvailable()) {
      IMU.readGyroscope(x_giro, y_giro, z_giro);
    }

    if (IMU.magneticFieldAvailable()) {
      IMU.readMagneticField(x_mag, y_mag, z_mag);
    }

    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x_acel, y_acel, z_acel);
    }
    if (x_acel > 0.1) {
      x_acel = 100 * x_acel;
      degreesX = map(x_acel, 0, 97, 0, 90);
    }
    if (x_acel < -0.1) {
      x_acel = 100 * x_acel;
      degreesX = map(x_acel, 0, -100, 0, 90);
    }
    if (y_acel > 0.1) {
      y_acel = 100 * y_acel;
      degreesY = map(y_acel, 0, 97, 0, 90);
    }
    if (y_acel < -0.1) {
      y_acel = 100 * y_acel;
      degreesY = map(y_acel, 0, -100, 0, 90);
    }
    if (z_acel > 0.1) {
      z_acel = 100 * z_acel;
      degreesZ = map(z_acel, 0, 97, 0, 90);
    }
    if (z_acel < -0.1) {
      z_acel = 100 * z_acel;
      degreesZ = map(z_acel, 0, -100, 0, 90);
    }

    flagTimer = false;
  }

      Serial.print("S");
      Serial.print(" ");
      Serial.print(x_mag);
      Serial.print(" ");
      Serial.print(y_mag);
      Serial.print(" ");
      Serial.print(z_mag);
      Serial.print(" ");
      Serial.print(x_acel);
      Serial.print(" ");
      Serial.print(y_acel);
      Serial.print(" ");
      Serial.print(z_acel);
      Serial.print(" ");
      Serial.print(x_giro);
      Serial.print(" ");
      Serial.print(y_giro);
      Serial.print(" ");
      Serial.println(z_giro);

  timer.update();
}
