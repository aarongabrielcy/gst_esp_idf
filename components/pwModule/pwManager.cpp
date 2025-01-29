#include "pwManager.h"

// Encender el módulo SIM
void PwManager::powerModule() {
    gpio_reset_pin((gpio_num_t)POWER_SIM_PIN);
    ESP_LOGI("PwManager", "Inicializado pin POWER=%d", POWER_SIM_PIN);
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << POWER_SIM_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io_conf);

    // Asegurar el estado inicial del pin
    gpio_set_level((gpio_num_t)POWER_SIM_PIN, 1);
}

// Apagar el módulo SIM
void PwManager::OffModule() {
    gpio_set_direction((gpio_num_t)POWER_SIM_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)POWER_SIM_PIN, 0);
}

// Reiniciar el ESP32
void PwManager::restartMicro() {
    esp_restart();
}

// Activar el pin de encendido del módulo SIM
void PwManager::powerKey() {
    gpio_reset_pin((gpio_num_t)POWER_KEY_PIN);
    ESP_LOGI("PwManager", "Inicializado pin POWERKEY=%d", POWER_KEY_PIN);
     gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << POWER_KEY_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)POWER_KEY_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 segundo
    gpio_set_level((gpio_num_t)POWER_KEY_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(3000)); // Esperar 3 segundos
}

// Configurar el LED GNSS
void PwManager::powerLedGnss() {
    gpio_reset_pin((gpio_num_t)GNSS_LED_PIN);
    ESP_LOGI("PwManager", "Inicializado pin LEDGPS=%d", GNSS_LED_PIN);
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << GNSS_LED_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)GNSS_LED_PIN, 0); // Apagar inicialmente
}

// Controlar el parpadeo del LED GNSS
void PwManager::blinkLedGnss(int fixState) {
    unsigned long currentMillis = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if (currentMillis - previousMillis >= ledInterval) {
        previousMillis = currentMillis;

        // Cambiar el estado del LED
        ledState = (fixState == 0) ? !ledState : fixState;
        //ESP_LOGI("PwManager", "Estado LED_GPS=%d", ledState);
        gpio_set_level((gpio_num_t)GNSS_LED_PIN, ledState);
    }
}

// Inicializar el pin de ignición
void PwManager::initInIgn(int ignPin) {
    gpio_reset_pin((gpio_num_t)ignPin);
    ESP_LOGI("PwManager", "Inicializando pin de ignición=%d", ignPin);
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << ignPin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io_conf);
}

// Leer el estado de ignición
bool PwManager::getStateIgn() {
    return gpio_get_level((gpio_num_t)pinIgn);
}
