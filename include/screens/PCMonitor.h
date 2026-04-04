#ifndef PCMONITOR_H
#define PCMONITOR_H

#include <M5Unified.h>

// Funkcja rysująca ekran statystyk PC
void renderPCMonitorScreen(M5Canvas &sprite);

// Funkcja asynchronicznego pobierania danych z PC wywoływana z głównej pętli
void handlePCBackgroundFetch();

#endif // PCMONITOR_H
