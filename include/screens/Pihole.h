#ifndef PIHOLE_H
#define PIHOLE_H

#include <M5Unified.h>

// Ustawienia dla Pi-hole, wywoływane przy starcie programu (logowanie do sieci)
void initPiholeWiFi();

// Funkcja rysująca ekran monitora Pi-Hole
void renderPiholeScreen(M5Canvas &sprite);

// Funkcja ręcznego asynchronicznego pobierania danych z PiHole wywoływana z głównej pętli
void handlePiholeBackgroundFetch();

#endif // PIHOLE_H
