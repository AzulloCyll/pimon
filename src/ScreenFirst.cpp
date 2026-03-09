#include "ScreenFirst.h"

void updateFirstScreen(M5Canvas &sprite, const SensorData &data) {
    // Zmienne statyczne do odświeżania temperatury co 30 sekund
    static float lastDisplayedTemp = 0.0;
    static unsigned long lastTempUpdate = 0;
    
    // Zaktualizuj widoczną temperaturę tylko jeśli minęło 30 sekund lub to pierwsze uruchomienie
    if (millis() - lastTempUpdate >= 30000 || lastTempUpdate == 0) {
        const float TEMP_OFFSET = -4.7;
        lastDisplayedTemp = data.ambTempC + TEMP_OFFSET;
        lastTempUpdate = millis();
    }

    sprite.fillSprite(BLACK);
    
    // Ustawienie punktu odniesienia 
    sprite.setTextDatum(top_left);

    // --- Pasek tytułowy (w stylu Pi-hole) ---
    sprite.setTextColor(GREEN);
    sprite.setTextSize(1.5);
    sprite.drawString("TERMOMETR", 5, 5);
    sprite.drawFastHLine(0, 22, 128, WHITE);

    // --- Sekcja Temperatura Pokoju ---
    sprite.setTextColor(CYAN);
    sprite.setTextSize(1);
    // Lekko obniżamy względem paska 
    sprite.drawString("Temperatura:", 5, 35); 

    // Wyświetlamy samą wartość
    sprite.setTextColor(WHITE);
    sprite.setTextSize(3);  // Duży wyraźny tekst pomiaru
    
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f\xF7""C", lastDisplayedTemp);
    // Z pozycją na Y równą 50, oparta na top-left
    sprite.drawString(tempStr, 5, 50); 

    // Opcjonalnie: możemy dodać informację o tym czy czujnik wykrywa obecnie kogoś w pokoju
    // wykorzystując do tego wolne miejsce na dole!
    sprite.setTextColor(CYAN);
    sprite.setTextSize(1);
    sprite.drawString("Status obecnosci:", 5, 90);

    sprite.setTextSize(2);
    if (data.isPresent || data.isMoving) {
        sprite.setTextColor(GREEN);
        sprite.drawString("Wykryto", 5, 105);
    } else {
        sprite.setTextColor(0x7BEF); // Szary / Ciemny niebieski
        sprite.drawString("Brak", 5, 105);
    }

    sprite.pushSprite(0, 0); // Zrzut ramki na ekran
}
