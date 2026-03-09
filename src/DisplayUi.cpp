#include "DisplayUi.h"
#include "ScreenFirst.h"
#include "ScreenSecond.h"
#include "ScreenThird.h"

static M5Canvas sprite(&M5.Display);
static M5Canvas oldSprite(&M5.Display);
static bool spriteCreated = false;
static int lastScreenIndex = 0;

void initDisplay() {
    M5.Display.setRotation(1);
    M5.Display.fillScreen(BLACK);

    if (!spriteCreated) {
        sprite.createSprite(128, 128); // AtomS3 ekran
        oldSprite.createSprite(128, 128); // Bufor na stary ekran do animacji
        spriteCreated = true;
    }
}

void renderScreen(M5Canvas &targetSprite, const SensorData &data, int screenIndex) {
    if (screenIndex == 2) {
        updateThirdScreen(targetSprite);
    } else if (screenIndex == 1) {
        updateSecondScreen(targetSprite);
    } else {
        updateFirstScreen(targetSprite, data);
    }
}

void updateDisplay(const SensorData &data, int screenIndex) {
    if (screenIndex != lastScreenIndex) {
        // Animacja przesunięcia
        // 1. Zapisz obecny widok starego ekranu
        renderScreen(oldSprite, data, lastScreenIndex);
        
        // 2. Przygotuj nowy ekran w głównym sprite
        renderScreen(sprite, data, screenIndex);
        
        // 3. Wykonaj animację przejścia (przesuwanie w lewo/prawo)
        int step = 16; // Krok o 16 pikseli (8 klatek animacji, całkiem płynne i szybkie)
        bool movingRight = screenIndex > lastScreenIndex;
        // Skok z 2 na 0 -> potraktuj jako ruch "w prawo" do przodu
        if (lastScreenIndex == 2 && screenIndex == 0) movingRight = true;
        // Skok z 0 na 2 -> "w lewo" do tyłu
        if (lastScreenIndex == 0 && screenIndex == 2) movingRight = false;
        
        for (int i = 0; i <= 128; i += step) {
            if (movingRight) {
                // Nowy wsuwa się z prawej strony
                oldSprite.pushSprite(-i, 0); 
                sprite.pushSprite(128 - i, 0);
            } else {
                // Nowy wsuwa się z lewej strony
                oldSprite.pushSprite(i, 0); 
                sprite.pushSprite(-128 + i, 0);
            }
            // delay(5); // Zakomentowane: sam czas renderowania 2 sprite'ów ustali FPS (przeważnie na poziomie ESP to będzie dobre tempo)
        }
        lastScreenIndex = screenIndex;
    } else {
        // Zwykła aktualizacja aktywnego ekranu bez animacji
        renderScreen(sprite, data, screenIndex);
    }
    
    // Zrzut zaktualizowanej klatki
    sprite.pushSprite(0, 0);
}
