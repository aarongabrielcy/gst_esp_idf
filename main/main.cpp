#include "SIM7600.h"
#include <string>
#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pwManager.h"
#include "system_config.h"


PwManager pwManager;
SIM7600 simModule(UART_NUM_1);  // Usamos UART1 para la comunicación con el SIM7600

extern "C" void app_main() {
    esp_err_t ret = gpio_install_isr_service(0);
    if (ret != ESP_OK) {
        ESP_LOGE("GPIO", "Error instalando ISR service: %s", esp_err_to_name(ret));
    }
    pwManager.powerModule();
    pwManager.powerKey();
    pwManager.powerLedGnss();
    pwManager.initInIgn(INPUT_IGN);
    simModule.begin();

    // Tarea para leer comandos del monitor serial
    char input[256];

    while (true) {
        bool ignState = pwManager.getStateIgn() ? 0 : 1;
        pwManager.blinkLedGnss(ignState);

        //printf("Ingrese un comando AT: ");
        if (fgets(input, sizeof(input), stdin)) {
            // Eliminar el salto de línea '\n' al final de la entrada
            input[strcspn(input, "\n")] = 0;

            // Enviar el comando AT al módulo
            simModule.sendATCommand(input);

            // Leer y mostrar la respuesta
            printf("Esperando respuesta...\n");
            std::string response = simModule.readResponse();
            printf("Response: %s\n", response.c_str());
        }

        vTaskDelay(pdMS_TO_TICKS(SYSTEM_TASK_DELAY_MS)); // Espera para evitar saturar la CPU
    }
}
