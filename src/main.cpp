#include <M5Unified.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "SensorTmos.h"
#include "DisplayUi.h"
#include "ScreenSecond.h"
#include "ScreenThird.h"
#include "secrets.h"

int currentScreen = 0;
const int maxScreens = 3; // ilość obsługiwanych ekranów

#ifndef PIO_UNIT_TESTING
void setup() {
    auto cfg = M5.config();
    // Rejestracja bazy pod spodem (Atomic Echo Base dla AtomS3 zostaje poprawnie przypisana)
    cfg.external_speaker.atomic_echo = true; 
    M5.begin(cfg);

    Serial.begin(115200);
    delay(100);

    // Głośność klikania (ustawiona na przyjemne, ciche 20)
    M5.Speaker.setVolume(20); 

    initTmosSensor();
    
    // Inicjalizacja polaczenia sieciowego dla PiHole (z timeoutem)
    initPiholeWiFi();
    
    initDisplay();
}

void loop() {
    M5.update();
    
    // Status aktualizacji - czy musimy przerysować ekran
    bool needUpdate = false;
    
    // Zmienne do obsługi wygaszania ekranu
    static unsigned long lastActivityTime = millis();
    static bool isScreenOn = true;
    const unsigned long SCREEN_TIMEOUT_MS = 3000; // 3 sekundy

    // Obsługa przycisku do zmiany ekranu i długiego kliknięcia
    static bool postSent = false;

    // Jeżeli trzyma on przycisk przez ponad 1 sekundę
    if (M5.BtnA.isPressed() && M5.BtnA.pressedFor(1000)) {
        if (!postSent) {
            M5.Speaker.tone(1200, 150); // Informacyjny dźwięk długi
            Serial.println("Długie naciśnięcie - wysyłam sygnał zamknięcia RPi...");
            if (WiFi.status() == WL_CONNECTED) {
                HTTPClient http;
                String shutdownUrl = String(SECRET_RPI_API_URL) + "/shutdown";
                http.begin(shutdownUrl); // URL wyłączenia Raspberry
                http.addHeader("Content-Type", "application/json");
                int httpCode = http.POST("{\"command\":\"poweroff\"}");
                Serial.printf("Sygnał zamknięcia wysłany, odpowiedź HTTP: %d\n", httpCode);
                http.end();
            }
            postSent = true; // Zabezpieczenie przed wielokrotnym wysyłaniem
            lastActivityTime = millis();
        }
    } else if (M5.BtnA.wasReleased()) {
        // Jeśli przycisk został puszczony, a POST nie został wysłany (było to krótkie kliknięcie)
        if (!postSent) {
            M5.Speaker.tone(2000, 40); 
            currentScreen = (currentScreen + 1) % maxScreens;
            lastActivityTime = millis(); 
            needUpdate = true;
        }
        postSent = false; // Reset dla kolejnych kliknięć
    }
    
    static SensorData lastSensorData = {0, 0, 0.0, 0.0, false, false};
    static bool prevWasMoving = false;
    
    // Zmienne do obsługi gestów (machnięcia ręką)
    static unsigned long lastSwipeTime = 0;
    const unsigned long SWIPE_COOLDOWN_MS = 1000; // Wydłużamy do 2 sekund
    
    if (readTmosData(lastSensorData)) {
        // Zamiast odświeżać ekran zawsze przy otrzymaniu danych z TMOS, wymuszamy 
        // przerysowanie tylko dla ekranu nr 0, który te dane bezpośrednio wyświetla.
        if (currentScreen == 0) {
            needUpdate = true;
        }
        
        // Wypisane do celów diagnostycznych, byś mógł podejrzeć w Monitorze dlaczego ekran się świeci
        // Zakomentowane aby nie "spamować" konsoli 15x na sekundę. Odkomentuj w razie debuggowania.
        // Serial.printf("P:%d M:%d\n", lastSensorData.isPresent, lastSensorData.isMoving);

        // Obsługa machnięcia ręką (zmiana ekranu) - reagujemy tylko na moment pojawienia się ruchu (zbocze narastające)
        if (lastSensorData.isMoving && !prevWasMoving && (millis() - lastSwipeTime > SWIPE_COOLDOWN_MS)) {
            M5.Speaker.tone(1000, 40); // Inny, cichy dźwięk na obudzenie/zmianę
            currentScreen = (currentScreen + 1) % maxScreens;
            lastSwipeTime = millis();
            lastActivityTime = millis();
            needUpdate = true;
            Serial.println("Wykryto machnięcie! Zmiana ekranu.");
        }
        prevWasMoving = lastSensorData.isMoving;
    } 

    // --- Obsługa obracania ekranu na podstawie żyroskopu (IMU) ---
    float ax, ay, az;
    if (M5.Imu.getAccelData(&ax, &ay, &az)) {
        int currentRotation = M5.Display.getRotation();
        int newRotation = currentRotation;

        // Określamy ułożenie na podstawie wektora grawitacji 
        // Histereza - sprawdzamy która oś jest wyraźnie dominująca
        // Różnica 0.15G zapobiega miganiu, gdy urządzenie jest trzymane pod kątem ~45 stopni
        if (abs(az) < 0.7) { 
            if (abs(ax) > abs(ay) + 0.15) {
                if (ax > 0) newRotation = 1;
                else newRotation = 3;
            } else if (abs(ay) > abs(ax) + 0.15) {
                if (ay > 0) newRotation = 0;
                else newRotation = 2;
            }
        }

        if (newRotation != currentRotation) {
            Serial.printf("Obrót zmiany: ax=%.2f ay=%.2f az=%.2f -> ROT=%d\n", ax, ay, az, newRotation);
            M5.Display.setRotation(newRotation);
            needUpdate = true; // Wymuś odświeżenie grafiki pod nowym kątem
            lastActivityTime = millis(); // Obrót traktujemy jako aktywność wybudzającą wygaszacz
        }
    }

    // Jeśli używamy ekranu nr 1 (Pi-Hole) lub nr 2 (Raspberry), statystyki w tle odświeżają się 
    // bardzo rzadko (1-10 sekund). Nie ma sensu przerysowywać wyświetlacza aż 15 razy na sekundę.
    static unsigned long lastStaticScreenUpdate = 0;
    if (currentScreen != 0 && (millis() - lastStaticScreenUpdate > 1000)) {
        needUpdate = true;
        lastStaticScreenUpdate = millis();
    }

    // Rejestrowanie aktywności z TMOS - jeśli to Ty patrzysz na ekran, `isPresent` nie pozwoli mu zgasnąć!
    // Możesz na próbę odłączyć `.isPresent` z tego "ifa", by gasło zawsze po bezruchu.
    if (lastSensorData.isPresent || lastSensorData.isMoving) {
        lastActivityTime = millis();
    }

    // -- Pobieranie z sieci PiHole dziala w ukryciu jako stale narzedzie niezaleznie od wybranego ekranu
    handlePiholeBackgroundFetch();
    
    // -- Pobieranie statystyk malinki również co określoną ilość sekund w tle
    handleRaspberryBackgroundFetch();

    // Logika włączania i wyłączania (wygaszania) ekranu
    if (millis() - lastActivityTime > SCREEN_TIMEOUT_MS) {
        if (isScreenOn) {
            M5.Display.sleep(); // Bezpieczniejsze API do twardego uśpienia wyświetlacza 
            // M5.Display.setBrightness(0); 
            isScreenOn = false;
        }
    } else {
        if (!isScreenOn) {
            M5.Display.wakeup(); // Wybudzenie API wyświetlacza
            // M5.Display.setBrightness(255); 
            isScreenOn = true;
            needUpdate = true; // Po wybudzeniu zmuś UI do ponownego zrenderowania
        }
    }

    // Odśwież UI tylko gdy jest taka potrzeba ORAZ zgaszony ekran tego nie blokuje
    if (needUpdate && isScreenOn) {
        updateDisplay(lastSensorData, currentScreen);
    }
    
    // Krotki delay, nie chcemy blokowac petli
    delay(20); 
}
#endif
