#include "DisplayUi.h"
#include "ScreenFirst.h"
#include "ScreenSecond.h"
#include "ScreenThird.h"

static M5Canvas sprite(&M5.Display);
static bool spriteCreated = false;

void initDisplay() {
    M5.Display.setRotation(1);
    M5.Display.fillScreen(BLACK);

    if (!spriteCreated) {
        sprite.createSprite(128, 128); // AtomS3 ekran
        spriteCreated = true;
    }
}

void updateDisplay(const SensorData &data, int screenIndex) {
    if (screenIndex == 2) {
        // Trzeci ekran
        updateThirdScreen(sprite);
    } else if (screenIndex == 1) {
        // Drugi ekran
        updateSecondScreen(sprite);
    } else {
        // Ekran domyślny (pierwszy, screenIndex == 0)
        updateFirstScreen(sprite, data);
    }
}
