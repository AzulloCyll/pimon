#ifndef SENSOR_TMOS_H
#define SENSOR_TMOS_H

#include <M5Unified.h>
#include <M5_STHS34PF80.h>

extern M5_STHS34PF80 tmos;

// Funkcja inicjalizująca i konfigurująca czujnik
void initTmosSensor();

// Zwraca true, jeśli czujnik został poprawnie wykryty
bool isSensorAvailable();

// Struktura przechowująca najświeższe dane z czujnika
struct SensorData {
    int16_t presenceVal;
    int16_t motionVal;
    float objTempC;
    float ambTempC;
    bool isPresent;
    bool isMoving;
    bool isAvailable;
};

// Funkcja sprawdzająca czy są nowe dane i aktualizująca podaną strukturę
bool readTmosData(SensorData &data);

#endif // SENSOR_TMOS_H
