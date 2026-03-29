#ifndef THERMOMETER_H
#define THERMOMETER_H

#include <M5Unified.h>
#include "SensorTmos.h"

// Funkcja rysująca ekran termometru (główny ekran z temperaturą)
void renderThermometerScreen(M5Canvas &sprite, const SensorData &data);

#endif // THERMOMETER_H
