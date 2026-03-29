#include "SensorTmos.h"

M5_STHS34PF80 tmos;
static bool isTmosSensorPresent = false;

void initTmosSensor() {
    Serial.println("Initialising TMOS Sensor...");

    if (!tmos.begin(&Wire, STHS34PF80_I2C_ADDRESS, 2, 1)) {
        Serial.println("TMOS sensor not found!");
        M5.Display.setTextColor(RED);
        M5.Display.println("TMOS Sensor NOT found!");
        isTmosSensorPresent = false;
        return; // Don't hang, just exit init
    }
    
    if (tmos.init() != 0) {
        Serial.println("Sensor Init Failed!");
        isTmosSensorPresent = false;
        return;
    }

    isTmosSensorPresent = true;

    tmos.setTmosODR(STHS34PF80_TMOS_ODR_AT_15Hz);
    tmos.setMotionThreshold(0xC8);  // Default z instrukcji M5Stack
    tmos.setPresenceThreshold(0xC8);  // Default z instrukcji M5Stack
    tmos.setPresenceHysteresis(0x32);
    tmos.setMotionHysteresis(0x32);
    
    // Obniżony gain zapobiega fałszywym odczytom stałej obecności
    tmos.setGainMode(STHS34PF80_GAIN_WIDE_MODE);

    tmos.resetAlgo(); // Kluczowe, by wyczyścić filtr obecności przy starcie 
    tmos.reset();
    
    delay(1000); // Wait for the sensor to stabilize
    Serial.println("TMOS sensor initialised.");
}

bool isSensorAvailable() {
    return isTmosSensorPresent;
}

bool readTmosData(SensorData &data) {
    data.isAvailable = isTmosSensorPresent;
    if (!isTmosSensorPresent) return false;

    sths34pf80_tmos_drdy_status_t drdy;
    if (tmos.getDataReady(&drdy) == 0 && drdy.drdy) {
        sths34pf80_tmos_func_status_t status;
        int16_t ambTempRaw = 0;

        tmos.getStatus(&status);
        tmos.getPresenceValue(&data.presenceVal);
        tmos.getMotionValue(&data.motionVal);
        tmos.getTemperatureData(&data.objTempC);
        tmos.getTAmbientRawValue(&ambTempRaw);

        data.ambTempC = (float)ambTempRaw / 100.0f;
        data.isPresent = status.pres_flag;
        data.isMoving = status.mot_flag;

        return true;
    }
    return false;
}
