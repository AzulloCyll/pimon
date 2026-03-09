#ifndef SENSOR_TMOS_H
#define SENSOR_TMOS_H

#include <M5Unified.h>
#include <M5_STHS34PF80.h>

extern M5_STHS34PF80 tmos;

// Funkcja inicjalizująca i konfigurująca czujnik
void initTmosSensor();

// Struktura przechowująca najświeższe dane z czujnika
struct SensorData {
    int16_t presenceVal;
    int16_t motionVal;
    float objTempC;
    float ambTempC;
    bool isPresent;
    bool isMoving;
};

// Funkcja sprawdzająca czy są nowe dane i aktualizująca podaną strukturę
bool readTmosData(SensorData &data);

#endif // SENSOR_TMOS_H
