#include "SIM7600.h"
#include <string>
#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pwManager.h"
#include "system_config.h"
#include "tcp.h"
#include "gpsManager.h"

SIM7600 simModule(UART_NUM_1);  // Usamos UART1 para la comunicación con el SIM7600
PwManager pwManager;
tcp TCP(simModule);
gpsManager GPS(simModule);

void gps_task(void *pvParameters) {
    char buffer[256];
    while (true) {
        if (xQueueReceive(simModule.gps_queue, buffer, portMAX_DELAY)) {
            printf("GPS Data:%s\n", buffer);
            // Aquí puedes parsear y enviar a tu servidor TCP
        }
    }
}

void network_task(void *pvParameters) {
    char buffer[256];
    while (true) {
        /*vTaskDelay(pdMS_TO_TICKS(2000));  // Cada 2 segundos pedir datos
        simModule.readUART();  // Leer datos entrantes*/
        if (xQueueReceive(simModule.network_queue, buffer, portMAX_DELAY)) {
            printf("Network Data:%s\n", buffer);
            // Procesar datos de red y enviarlos al servidor
        }
    }
}
void sms_task(void *pvParameters) {
    SMSData sms;
    while (true) {
        if (xQueueReceive(simModule.sms_queue, &sms, portMAX_DELAY)) {
            ESP_LOGI("SMS_TASK", "SMS recibido:");
            ESP_LOGI("SMS_TASK", "De: %s", sms.phone_sms.c_str());
            ESP_LOGI("SMS_TASK", "Fecha: %s", sms.datetime_sms.c_str());
            ESP_LOGI("SMS_TASK", "Mensaje: %s", sms.data_sms.c_str());
        }//BORRAR SMS DESPUES DE PROCESAR EL "MENSAJE", "DESENCADENAR LA ACCION SEGUN EL COMANDO" Y DESPUÉS DE EJECUTAR LA ACCIÓN ELIMNIAR EL SMS CON EL MISMO INDEX
    }
}
void event_task(void *pvParameters) {
    char buffer[256];
    while (true) {
        if (xQueueReceive(simModule.evt_queue, &buffer, portMAX_DELAY)) {
            ESP_LOGI("EVENT_TASK", "Evento recibido:%s", buffer);
             // Convertir buffer a número
            char *endPtr;
            int index = strtol(buffer, &endPtr, 10);
 
            if (*endPtr == '\0' && index > 0) {  // Verifica que es un número válido
                char readCmd[32];
                snprintf(readCmd, sizeof(readCmd), "AT+CMGR=%d", index);
                simModule.sendATCommand(readCmd);
                ESP_LOGI("EVENT_TASK", "Leyendo SMS con comando: %s", readCmd);
            } else {
                 ESP_LOGW("EVENT_TASK", "Índice inválido, ignorado.");
            }
        }
    }
}
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
    TCP.activeTcpService();
    TCP.configTcpServer(SERVER_URL, SERVER_PORT);
    GPS.activeGps(1);
    GPS.confiGpsReports(1);

    xTaskCreate(gps_task, "gps_task", 4096, NULL, 5, NULL);
    xTaskCreate(network_task, "network_task", 4096, NULL, 5, NULL);
    xTaskCreate(sms_task, "sms_task", 4096, NULL, 5, NULL);
    xTaskCreate(event_task, "event_task", 4096, NULL, 5, NULL);
    // Tarea para leer comandos del monitor serial
    char input[256];

    while (true) {
        bool ignState = pwManager.getStateIgn() ? 0 : 1;
        pwManager.blinkLedGnss(ignState);

        //printf("Ingrese un comando AT: ");
        if (fgets(input, sizeof(input), stdin)) {
            // Eliminar el salto de línea '\n' al final de la entrada
            input[strcspn(input, "\n")] = 0;
            simModule.sendATCommand(input);
            // Enviar el comando AT al módulo
            /*std::string response = simModule.sendCommandWithResponse(input, SYSTEM_INIT_DELAY_MS);
            ESP_LOGI("MAIN", "Respuesta AT: %s", response.c_str());*/
        }
        simModule.readUART();  // Leer datos entrantes
        vTaskDelay(pdMS_TO_TICKS(SYSTEM_TASK_DELAY_MS)); // Espera para evitar saturar la CPU
    }
}
