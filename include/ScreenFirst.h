#ifndef SCREEN_FIRST_H
#define SCREEN_FIRST_H

#include <M5Unified.h>
#include "SensorTmos.h"

// Funkcja rysująca pierwszy ekran (główny ekran z temperaturą)
void updateFirstScreen(M5Canvas &sprite, const SensorData &data);

#endif // SCREEN_FIRST_H
