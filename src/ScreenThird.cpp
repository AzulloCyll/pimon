#include "ScreenThird.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

String rpi_api_url = String(SECRET_RPI_API_URL) + "/stats";

struct RaspberryData {
    float cpu = 0.0;
    float ram = 0.0;
    float temp = 0.0;
    int lastHttpCode = 0;
};

static RaspberryData cachedRpiData;
static unsigned long lastRpiUpdate = 0;

void fetchRaspberryData() {
    if (WiFi.status() != WL_CONNECTED) {
        cachedRpiData.lastHttpCode = -1; // Brak WiFi
        return;
    }

    HTTPClient http;
    http.begin(rpi_api_url);
    http.setTimeout(3000); // 3 sekundy timeoutu uderzenia w API
    
    int httpCode = http.GET();
    cachedRpiData.lastHttpCode = httpCode;
    
    if (httpCode == 200) {
        DynamicJsonDocument doc(1024);
        
        // Zabezpieczenie przed uszkodzonym JSONem oraz strumieniowe paroswanie
        DeserializationError error = deserializeJson(doc, http.getStream());
        if (!error) {
            // Zakładam tu domyślne nazwy właściwości w JSON, takie jak "cpu", "ram", "temp"
            // Jeśli twoje API używa innych nazw, zmień je poniżej!
            cachedRpiData.cpu = doc["cpu"] | 0.0;
            cachedRpiData.ram = doc["ram"] | 0.0;
            cachedRpiData.temp = doc["temp"] | 0.0;
        } else {
            cachedRpiData.lastHttpCode = -4; // Błąd parsowania JSON
        }
    }
    
    http.end();
}

void rpiFetchLoop(void* parameter) {
    while(true) {
        fetchRaspberryData();
        // Czekaj 5 sekund bez zabijania wątku
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void handleRaspberryBackgroundFetch() {
    static bool taskStarted = false;
    if (!taskStarted) {
        xTaskCreate(
            rpiFetchLoop, 
            "RpiFetchTask", 
            4096,              
            NULL,              
            1,                 
            NULL               
        );
        taskStarted = true;
    }
}

void updateThirdScreen(M5Canvas &sprite) {
    sprite.fillSprite(BLACK);
    sprite.setTextDatum(top_left);

    // --- Pasek tytułowy ---
    sprite.setTextColor(GREEN);
    sprite.setTextSize(1.5);
    sprite.drawString("RASPBERRY PI", 5, 5);
    sprite.drawFastHLine(0, 22, 128, WHITE);

    if (cachedRpiData.lastHttpCode == 200 || cachedRpiData.lastHttpCode == 0) {
        // CPU
        sprite.setTextColor(CYAN);
        sprite.setTextSize(1);
        sprite.drawString("Uzycie CPU:", 5, 30);
        
        sprite.setTextColor(WHITE);
        sprite.setTextSize(1.5);
        char cpuStr[16];
        snprintf(cpuStr, sizeof(cpuStr), "%.1f %%", cachedRpiData.cpu);
        sprite.drawString(cpuStr, 5, 45);

        // RAM
        sprite.setTextColor(CYAN);
        sprite.setTextSize(1);
        sprite.drawString("Uzycie RAM:", 5, 65);
        
        sprite.setTextColor(WHITE);
        sprite.setTextSize(1.5);
        char ramStr[16];
        snprintf(ramStr, sizeof(ramStr), "%.1f %%", cachedRpiData.ram);
        sprite.drawString(ramStr, 5, 80);

        // TEMP
        sprite.setTextColor(CYAN);
        sprite.setTextSize(1);
        sprite.drawString("Temperatura:", 5, 100);
        
        sprite.setTextColor(WHITE);
        sprite.setTextSize(1.5);
        char tempStr[16];
        snprintf(tempStr, sizeof(tempStr), "%.1f \xF7""C", cachedRpiData.temp);
        sprite.drawString(tempStr, 5, 115);
        
    } else if (cachedRpiData.lastHttpCode == -1) {
        sprite.fillSprite(RED);
        sprite.setTextColor(WHITE);
        sprite.setTextSize(1.5);
        sprite.drawString("Brak WiFi", 5, 40);
    } else {
        sprite.fillSprite(RED);
        sprite.setTextColor(WHITE);
        sprite.setTextDatum(middle_center);
        
        // Wyświetlanie konkretnego błędu na środku ekranu
        if(cachedRpiData.lastHttpCode == -4) {
            sprite.setTextSize(1.5);
            sprite.drawString("Blad JSON", 64, 50);
            sprite.setTextSize(2);
            sprite.drawString("ERR:-4", 64, 75);
        } else if(cachedRpiData.lastHttpCode < 0) {
            // Ujemne kody (np. -1 to ERR_CONNECTION_REFUSED z biblioteki HTTPClient)
            sprite.setTextSize(1.5);
            sprite.drawString("Malina Offline", 64, 50);
            sprite.setTextSize(2);
            char errStr[16];
            snprintf(errStr, sizeof(errStr), "ERR:%d", cachedRpiData.lastHttpCode);
            sprite.drawString(errStr, 64, 75);
        } else {
            // Dodatnie kody HTTP (np. 404, 500)
            sprite.setTextSize(1.5);
            sprite.drawString("Blad Serwera", 64, 50);
            sprite.setTextSize(2);
            char errStr[16];
            snprintf(errStr, sizeof(errStr), "HTTP:%d", cachedRpiData.lastHttpCode);
            sprite.drawString(errStr, 64, 75);
        }
        
        sprite.setTextDatum(top_left); // Przywrócenie domyślnego
    }

    // "load..." 
    if (cachedRpiData.lastHttpCode == 0) {
        sprite.setTextDatum(bottom_right); 
        sprite.setTextSize(1);
        sprite.setTextColor(YELLOW);
        sprite.drawString("load...", 124, 124); 
        sprite.setTextDatum(top_left); 
    }

    // Usunięto sprite.pushSprite(0, 0); 
}
