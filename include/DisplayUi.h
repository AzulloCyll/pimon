#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <M5Unified.h>
#include "SensorTmos.h"

// Inicjalizacja ekranu i sprite'a
void initDisplay();

// Aktualizacja wskazań na ekranie na podstawie dostarczonych danych oraz wybranego ekranu
void updateDisplay(const SensorData &data, int screenIndex);

#endif // DISPLAY_UI_H
