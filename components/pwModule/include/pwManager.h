#ifndef PW_MODULE_H
#define PW_MODULE_H

#include "hardware_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
class PwManager {
private:
    unsigned long previousMillis = 0;
    unsigned long ledInterval = 200; // Intervalo para el parpadeo del LED
    int ledState = 0;                // Estado actual del LED
    int pinIgn;                      // Pin de ignición
public:
    // Métodos públicos
    void powerKey();
    void powerModule();
    void powerLedGnss();
    void blinkLedGnss(int fixState);
    void initInIgn(int pin);
    bool getStateIgn();
    void OffModule();
    void restartMicro();
};

#endif // PW_MODULE_H
