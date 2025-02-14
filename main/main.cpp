#include "SIM7600.h"
#include <string>
#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pwManager.h"
#include "initConfig.h"
#include "system_config.h"
#include "tcp.h"
#include "antennaInfo.h"
#include "gnssInfo.h"
#include "gprsManager.h"

SIM7600 simModule(UART_NUM_1);  // Usamos UART1 para la comunicación con el SIM7600
PwManager pwManager;
tcp TCP(simModule);
initConfig init(simModule);
gprsManager gprs;

antennaInfo cell;
gnssInfo gnss;

antennaInfo tempGprs;
gnssInfo tempGnss;

/// ESTAS TAREAS SOLO COMPRUEBAN QUE LOS DATOS ESTÁN PARSEADOS CORRECTAMENTE BORRAR DESPUÉS
void gps_task(void *pvParameters) {
    while (true) {
        if (xQueueReceive(simModule.gps_queue, &gnss, portMAX_DELAY)) {
            tempGnss = gnss;
            /*ESP_LOGI("GNSS_TASK", "Mode: %d, LAT: %f, NS: %c, LON: %f, EW: %c, DATE: %s, TIME: %s, SAT: %d, , FIX: %d",
                gnss.mode, gnss.lat, gnss.ns, gnss.log, gnss.ew, gnss.date.c_str(), gnss.utc_time.c_str(), gnss.gps_svs, gnss.fix);*/
        }
    }
}
void cell_task(void *pvParameters) {
    //char buffer[256];
    while (true) {
        if (xQueueReceive(simModule.cell_queue, &cell, portMAX_DELAY)) {
            tempGprs = cell;
           /*ESP_LOGI("CELL_TASK", "Mode: %s, MCC: %s, MNC: %s, LAC: %s, Cell ID: %s, Rx Level: %d",
            tempGprs.systemMode.c_str(), tempGprs.mcc.c_str(), tempGprs.mnc.c_str(),tempGprs.lac.c_str(), tempGprs.cellId.c_str(), tempGprs.rxLevel);*/
        }
    }
}
/*void cell_task(void *pvParameters) {
    char buffer[256];
    while (true) {
        if (xQueueReceive(simModule.cell_queue, buffer, portMAX_DELAY)) {
            printf("Cellular Data:%s\n", GPRS.parseCPSI(buffer).c_str() );
            // Procesar datos de red y enviarlos al servidor
        }
    }
}*/
void send_task(void *pvParameters) {
    while (true) {
        TickType_t lastWakeTime = xTaskGetTickCount(); // Guardar el tiempo actual
        /*ESP_LOGI("SEND_TASK", "Mode: %s, MCC: %s, MNC: %s, LAC: %s, Cell ID: %s, Rx Level: %d",
            tempGprs.systemMode.c_str(), tempGprs.mcc.c_str(), tempGprs.mnc.c_str(),tempGprs.lac.c_str(), tempGprs.cellId.c_str(), tempGprs.rxLevel);
        ESP_LOGI("SEND_TASK", "Mode: %d, LAT: %f, NS: %c, LON: %f, EW: %c, DATE: %s, TIME: %s, SAT: %d, , FIX: %d",tempGnss.mode, tempGnss.lat, tempGnss.ns, tempGnss.log, tempGnss.ew, tempGnss.date.c_str(), tempGnss.utc_time.c_str(), tempGnss.gps_svs, tempGnss.fix);*/
        //<HEAD>,<IMEI>,<MODEL>,<SW_VER><MSG_TYPE>,<DATE><TIME>,<CELL_ID>,<MCC>,<MNC>,<LAC>,<RX_LVL>,<LAT>,<N/S>,<LOG>,<E/W>,<SPD>,<CRS>,<SAT>,<FIX>,<IN_STATE>,<OUT_STATE>,<BCK_VOLT>,<PWR_VOLT>
        std::string sendData = std::string(Headers::STT)+DLM+sm.imei+DLM+tempGnss.date +DLM+tempGnss.utc_time+DLM+tempGprs.cellId+DLM+tempGprs.mcc+DLM+tempGprs.mnc+DLM+tempGprs.lac+DLM+std::to_string(tempGprs.rxLevel)+DLM+
        std::to_string(tempGnss.lat)+DLM+std::string(1,tempGnss.ns)+DLM+std::to_string(tempGnss.log)+DLM+std::string(1, tempGnss.ew)+DLM + std::to_string(tempGnss.gps_svs)+DLM+std::to_string(tempGnss.fix);
        ESP_LOGI("SEND_TASK", "Sending Data: %s", sendData.c_str()); 
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(20000));
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
        }//BORRAR SMS DESPUES DE PROCESAR EL "MENSAJE", "DESENCADENAR LA ACCION SEGUN EL COMANDO" Y DESPUÉS DE EJECUTAR LA ACCIÓN ELIMNIAR EL SMS CON EL MISMO INDEX o puede ser después de leer el msj
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
 
            if (*endPtr == '\0' && index >= 0) {  // Verifica que es un número válido
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
    init.activeGps(1);
    init.gpsReport(10);
    init.gprsReport(5);

    xTaskCreate(gps_task, "gps_task", 4096, NULL, 5, NULL);
    xTaskCreate(cell_task, "cell_task", 4096, NULL, 5, NULL);
    xTaskCreate(sms_task, "sms_task", 4096, NULL, 5, NULL);
    xTaskCreate(event_task, "event_task", 4096, NULL, 5, NULL);
    xTaskCreate(send_task, "send_task", 4096, NULL, 5, NULL);
    // Tarea para leer comandos del monitor serial
    char input[256];

    while (true) {
        bool ignState = pwManager.getStateIgn() ? 0 : 1;
        pwManager.blinkLedGnss(ignState);

        //printf("Ingrese un comando AT: ");
        if (fgets(input, sizeof(input), stdin)) {
            // Eliminar el salto de línea '\n' al final de la entrada
            input[strcspn(input, "\n")] = 0;
            simModule.sendATCommand(input);        }
        simModule.readUART();  // Leer datos entrantes
        vTaskDelay(pdMS_TO_TICKS(SYSTEM_TASK_DELAY_MS)); // Espera para evitar saturar la CPU
    }
}
