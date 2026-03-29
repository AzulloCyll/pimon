#ifndef RASPBERRY_H
#define RASPBERRY_H

#include <M5Unified.h>

// Funkcja rysująca ekran statystyk Raspberry Pi
void renderRaspberryScreen(M5Canvas &sprite);

// Funkcja asynchronicznego pobierania danych z RPi wywoływana z głównej pętli
void handleRaspberryBackgroundFetch();

#endif // RASPBERRY_H
