#include "ScreenSecond.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

const char* ssid = SECRET_WIFI_SSID;
const char* password = SECRET_WIFI_PASS;
const char* pihole_ip = SECRET_PIHOLE_IP;
const char* app_password = SECRET_PIHOLE_PASS;

static String sid = "";

// Struktura na dane pi-hole buforowane pomiędzy uderzeniami sieciowymi
struct PiholeData {
    long blocked = 0;
    long total = 0;
    float percent = 0.0;
    int lastHttpCode = 0;
};
static PiholeData cachedPiholeData;
static unsigned long lastPiholeUpdate = 0;

void initPiholeWiFi() {
    Serial.println("Connecting to WiFi...");
    
    // Konieczne wymuszenie poprawnej orientacji przed rysowaniem pierwszych pakietow diagnostycznych
    M5.Display.setRotation(1);
    
    // Renderowanie małego napisu o włączaniu do WiFi nim program wystartuje pętlę na dobre
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(1.5);
    M5.Display.drawString("WiFi...", 64, 64);
    
    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();
    
    // Czekaj na połączenie z timeout'em by nie zablokować urządzenia jak braknie routera
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
        delay(100); // Mniejsze delaye, częstsza weryfikacja
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        // Krótki komunikat o sukcesie widoczny przed wejściem w standardowe ekrany po rotacji
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextColor(GREEN);
        M5.Display.drawString("Online!", 64, 64);
        delay(500); 
    } else {
        Serial.println("\nWiFi connection failed! PiHole monitor won't work.");
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextColor(RED);
        M5.Display.drawString("Brak WiFi", 64, 64);
        delay(500); 
    }
}

int loginToPihole() {
    HTTPClient http;
    String url = "http://" + String(pihole_ip) + "/api/auth";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    String payload = "{\"password\":\"" + String(app_password) + "\"}";
    int httpCode = http.POST(payload);
    
    if (httpCode == 200) {
        String response = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, response);
        sid = doc["session"]["sid"].as<String>();
        http.end();
        return httpCode;
    }
    Serial.printf("Blad logowania: %d\n", httpCode);
    http.end();
    return httpCode;
}

void fetchPiholeData() {
    if (WiFi.status() != WL_CONNECTED) {
        cachedPiholeData.lastHttpCode = -1; // Brak sieci
        return;
    }

    if (sid == "" || sid == "null") {
        int authCode = loginToPihole();
        if (authCode != 200) {
            // Zapisujemy rzeczywisty kod błędu HTTP (np. 401, -1) zamiast -2
            cachedPiholeData.lastHttpCode = authCode == -1 ? -3 : authCode;
            return;
        }
    }

    HTTPClient http;
    String url = "http://" + String(pihole_ip) + "/api/stats/summary";
    http.begin(url);
    http.addHeader("X-FTL-SID", sid);
    
    int httpCode = http.GET();
    cachedPiholeData.lastHttpCode = httpCode;
    
    if (httpCode == 200) {
        DynamicJsonDocument doc(2048);
        // OPTYMALIZACJA: Bezpośrednie parsowanie ze strumienia sieciowego
        deserializeJson(doc, http.getStream());

        cachedPiholeData.blocked = doc["queries"]["blocked"];
        cachedPiholeData.total = doc["queries"]["total"];
        
        if (cachedPiholeData.total > 0) {
            cachedPiholeData.percent = ((float)cachedPiholeData.blocked / (float)cachedPiholeData.total) * 100.0;
        } else {
            cachedPiholeData.percent = 0.0;
        }
    } else if (httpCode == 401) {
        sid = ""; // Sesja wygasła
    }
    
    http.end();
}

void piholeFetchLoop(void* parameter) {
    while(true) {
        fetchPiholeData();
        // Czekaj 60 sekund w trybie uśpienia wątku - zadanie nigdy nie umiera,
        // przez co zapobiegamy fragmentacji pamięci RAM (Heap Fragmentation).
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}

void handlePiholeBackgroundFetch() {
    // Zostawiliśmy starą nazwę dla spójności z `main.cpp`, ale teraz
    // funkcja tylko JEDEN RAZ odpala permanentny wątek na całe życie programu.
    static bool taskStarted = false;
    if (!taskStarted) {
        xTaskCreate(
            piholeFetchLoop, 
            "PiholeFetchTask", 
            4096,              
            NULL,              
            1,                 
            NULL               
        );
        taskStarted = true;
    }
}

void updateSecondScreen(M5Canvas &sprite) {
    // Krok rysowania szkieletu / buforowanych danych: błyskawiczne renderowanie natychmiast po kliknięciu
    sprite.fillSprite(BLACK);
    sprite.setTextDatum(top_left);

    if (cachedPiholeData.lastHttpCode == 200 || cachedPiholeData.lastHttpCode == 0) {
        sprite.setTextColor(GREEN);
        sprite.setTextSize(1.5);
        sprite.drawString("PI-HOLE LIVE", 5, 5);
        sprite.drawFastHLine(0, 22, 128, WHITE);
        
        sprite.setTextColor(CYAN);
        sprite.setTextSize(1);
        sprite.drawString("Zablokowane:", 5, 35); // Było 30, zmiana na 35
        
        sprite.setTextColor(WHITE);
        sprite.setTextSize(2.5);
        char blockedStr[16];
        snprintf(blockedStr, sizeof(blockedStr), "%ld", cachedPiholeData.blocked);
        sprite.drawString(blockedStr, 5, 50); // Było 45, zmiana na 50
        
        sprite.setTextColor(CYAN);
        sprite.setTextSize(1);
        sprite.drawString("Zablokowano (%):", 5, 85); // Było 80, zmiana na 85
        
        sprite.setTextColor(WHITE);
        sprite.setTextSize(2.5);
        char percentStr[16];
        snprintf(percentStr, sizeof(percentStr), "%.1f%%", cachedPiholeData.percent);
        sprite.drawString(percentStr, 5, 100); // Było 95, zmiana na 100
        
    } else if (cachedPiholeData.lastHttpCode == -1) {
        sprite.fillSprite(RED);
        sprite.setTextColor(WHITE);
        sprite.setTextSize(1.5);
        sprite.drawString("Brak WiFi", 5, 40);
    } else {
        sprite.fillSprite(RED);
        sprite.setTextColor(WHITE);
        sprite.setTextSize(2);
        char errStr[16];
        snprintf(errStr, sizeof(errStr), "ERR:%d", cachedPiholeData.lastHttpCode);
        sprite.drawString(errStr, 5, 40);
    }

    // Rysujemy komunikat "load..." jeżeli dane układu zależą od zerowego ładowania
    if (cachedPiholeData.lastHttpCode == 0) {
        sprite.setTextDatum(bottom_right); 
        sprite.setTextSize(1);
        sprite.setTextColor(YELLOW);
        sprite.drawString("load...", 124, 124); 
        sprite.setTextDatum(top_left); 
    }
        
    // Usunięto sprite.pushSprite(0, 0); by zarządzać animacjami w DisplayUi
}
