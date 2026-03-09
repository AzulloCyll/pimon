#ifndef SCREEN_SECOND_H
#define SCREEN_SECOND_H

#include <M5Unified.h>

// Ustawienia dla Pi-hole, wywoływane przy starcie programu (logowanie do sieci)
void initPiholeWiFi();

// Funkcja rysująca drugi ekran (monitor Pi-Hole)
void updateSecondScreen(M5Canvas &sprite);

// Funkcja ręcznego asynchronicznego pobierania danych z PiHole wywoływana z głównej pętli
void handlePiholeBackgroundFetch();

#endif // SCREEN_SECOND_H
