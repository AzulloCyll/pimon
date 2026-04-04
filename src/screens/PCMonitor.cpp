#include "screens/PCMonitor.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

String pc_api_url = String(SECRET_PC_API_URL) + "/pc-stats";

struct PCData {
    float cpu = 0.0;
    float ram = 0.0;
    float temp = 0.0;
    int lastHttpCode = 0;
};

static PCData cachedPCData;

void fetchPCData() {
    if (WiFi.status() != WL_CONNECTED) {
        cachedPCData.lastHttpCode = -100; // Unikalny kod dla braku WiFi (PC)
        return;
    }

    HTTPClient http;
    http.begin(pc_api_url);
    http.setTimeout(3000); // 3 sekundy timeoutu
    
    int httpCode = http.GET();
    if (httpCode < 0) {
        Serial.printf("[PC Monitor] Blad HTTP: %d (%s)\n", httpCode, http.errorToString(httpCode).c_str());
    }
    cachedPCData.lastHttpCode = httpCode;
    
    if (httpCode == 200) {
        DynamicJsonDocument doc(1024);
        
        DeserializationError error = deserializeJson(doc, http.getStream());
        if (!error) {
            cachedPCData.cpu = doc["cpu"] | 0.0;
            cachedPCData.ram = doc["ram"] | 0.0;
            cachedPCData.temp = doc["temp"] | 0.0;
        } else {
            cachedPCData.lastHttpCode = -4; // Błąd parsowania JSON
        }
    }
    
    http.end();
}

void pcFetchLoop(void* parameter) {
    while(true) {
        fetchPCData();
        // Czekaj 5 sekund
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void handlePCBackgroundFetch() {
    static bool taskStarted = false;
    if (!taskStarted) {
        xTaskCreate(
            pcFetchLoop, 
            "PCFetchTask", 
            4096,              
            NULL,              
            1,                 
            NULL               
        );
        taskStarted = true;
    }
}

void renderPCMonitorScreen(M5Canvas &sprite) {
    sprite.fillSprite(BLACK);
    sprite.setTextDatum(top_left);

    // --- Pasek tytułowy - Zielony dla PC ---
    sprite.setTextColor(GREEN);
    sprite.setTextSize(1.5);
    sprite.drawString("MONITOR PC", 5, 5);
    sprite.drawFastHLine(0, 22, 128, WHITE);

    if (cachedPCData.lastHttpCode == 200 || cachedPCData.lastHttpCode == 0) {
        // CPU
        sprite.setTextColor(CYAN);
        sprite.setTextSize(1);
        sprite.drawString("Uzycie CPU:", 5, 30);
        
        sprite.setTextColor(WHITE);
        sprite.setTextSize(1.5);
        char cpuStr[16];
        snprintf(cpuStr, sizeof(cpuStr), "%.1f %%", cachedPCData.cpu);
        sprite.drawString(cpuStr, 5, 45);

        // RAM
        sprite.setTextColor(CYAN);
        sprite.setTextSize(1);
        sprite.drawString("Uzycie RAM:", 5, 65);
        
        sprite.setTextColor(WHITE);
        sprite.setTextSize(1.5);
        char ramStr[16];
        snprintf(ramStr, sizeof(ramStr), "%.1f %%", cachedPCData.ram);
        sprite.drawString(ramStr, 5, 80);

        // TEMP
        sprite.setTextColor(CYAN);
        sprite.setTextSize(1);
        sprite.drawString("Temp. GPU:", 5, 100);
        
        sprite.setTextColor(WHITE);
        sprite.setTextSize(1.5);
        char tempStr[16];
        snprintf(tempStr, sizeof(tempStr), "%.1f \xF7""C", cachedPCData.temp);
        sprite.drawString(tempStr, 5, 115);
        
    } else if (cachedPCData.lastHttpCode == -100) {
        sprite.fillSprite(RED);
        sprite.setTextColor(WHITE);
        sprite.setTextDatum(middle_center);
        sprite.setTextSize(1);
        sprite.drawString("WiFi (PC): BRAK", 64, 50);
        sprite.setTextSize(1);
        sprite.drawString("Polacz z rutera", 64, 75);
    } else {
        sprite.fillSprite(RED);
        sprite.setTextColor(WHITE);
        sprite.setTextDatum(middle_center);
        
        if(cachedPCData.lastHttpCode == -4) {
            sprite.setTextSize(1);
            sprite.drawString("Blad JSON (PC)", 64, 50);
            sprite.setTextSize(2);
            sprite.drawString("ERR:-4", 64, 75);
        } else if(cachedPCData.lastHttpCode == -1) {
            sprite.setTextSize(1);
            sprite.drawString("PC: OFFLINE", 64, 50);
            sprite.setTextSize(2);
            sprite.drawString("ERR:-1", 64, 75);
        } else if(cachedPCData.lastHttpCode == -11) {
            sprite.setTextSize(1);
            sprite.drawString("PC: TIMEOUT", 64, 50);
            sprite.setTextSize(2);
            sprite.drawString("ERR:-11", 64, 75);
        } else if(cachedPCData.lastHttpCode < 0) {
            sprite.setTextSize(1);
            sprite.drawString("Blad Sieci (PC)", 64, 50);
            sprite.setTextSize(1.5);
            char errStr[16];
            snprintf(errStr, sizeof(errStr), "ERR:%d", cachedPCData.lastHttpCode);
            sprite.drawString(errStr, 64, 75);
        } else {
            sprite.setTextSize(1);
            sprite.drawString("Blad HTTP (PC)", 64, 50);
            sprite.setTextSize(1.5);
            char errStr[16];
            snprintf(errStr, sizeof(errStr), "HTTP:%d", cachedPCData.lastHttpCode);
            sprite.drawString(errStr, 64, 75);
        }
        
        sprite.setTextDatum(top_left);
    }

    if (cachedPCData.lastHttpCode == 0) {
        sprite.setTextDatum(bottom_right); 
        sprite.setTextSize(1);
        sprite.setTextColor(YELLOW);
        sprite.drawString("load...", 124, 124); 
        sprite.setTextDatum(top_left); 
    }
}
