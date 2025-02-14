#include "SIM7600.h"
#include <string.h>
#include "utils/simUtils.h"
moduleInfo sm;

SIM7600::SIM7600(uart_port_t uart_num) : _uart_num(uart_num) {
    gps_queue = xQueueCreate(10, sizeof(gnssInfo) );
    cell_queue = xQueueCreate(10, sizeof(antennaInfo));
    sms_queue = xQueueCreate(10, sizeof(char) * 256);
    evt_queue = xQueueCreate(10, sizeof(char) * 256);
}

void SIM7600::begin() {
    // Configurar UART
    const uart_config_t uart_config = {
        .baud_rate = SIM7600_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_param_config(_uart_num, &uart_config);
    uart_set_pin(_uart_num, SIM7600_TX_PIN, SIM7600_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(_uart_num, 1024 * 2, 0, 0, NULL, 0);

    ESP_LOGI("SIM7600", "UART inicializado en pines TX=%d, RX=%d", SIM7600_TX_PIN, SIM7600_RX_PIN);
    if (!testUART()) {
        ESP_LOGE("SIM7600", "Fallo en la comunicación con el módulo SIM7600.");
    } else {
        ESP_LOGI("SIM7600", "Comunicación UART con SIM7600 establecida correctamente.");
    }
    getImei();
}
void  SIM7600::getImei() {
        sendATCommand("AT+SIMEI?");
}
bool SIM7600::testUART() {
    while (true) {
        sendATCommand("AT");
        vTaskDelay(pdMS_TO_TICKS(3000)); // Espera 500ms para la respuesta

        std::string response = readUART();
        if (response.find("OK") != std::string::npos) {
            ESP_LOGI("SIM7600", "Comunicación UART establecida.");
            return true;
        }
        ESP_LOGW("SIM7600", "No se recibió respuesta del SIM7600. Reintentando...");
    }
}
void SIM7600::sendATCommand(const std::string& command) {
    std::string atCommand = command + "\r\n";  // Agregar retorno de carro y nueva línea
    uart_write_bytes(_uart_num, atCommand.c_str(), atCommand.length());
    ESP_LOGI("SIM7600", "Comando AT enviado: %s", atCommand.c_str());
}
std::string SIM7600::readUART() {
    static std::string responseBuffer;
    static std::string atResponseBuffer; // Buffer para respuestas AT
    static bool capturingAT = false;     // Estado para saber si estamos capturando AT
    uint8_t data[256];

    // Leer datos del UART
    int len = uart_read_bytes(_uart_num, data, sizeof(data) - 1, pdMS_TO_TICKS(100));
    if (len > 0) {
        std::string receivedChunk(reinterpret_cast<char*>(data), len);
        ESP_LOGI("SIM7600", "Chunk recibido: [%s]", receivedChunk.c_str());

        responseBuffer += receivedChunk; // Acumulamos datos

        // Buscar líneas completas
        size_t pos = 0;
        while ((pos = responseBuffer.find("\n")) != std::string::npos) {
            std::string line = responseBuffer.substr(0, pos);
            responseBuffer.erase(0, pos + 1);  // Eliminar la línea procesada
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end()); // Limpiar '\r'

            //ESP_LOGI("SIM7600", "Línea procesada: [%s]", line.c_str());

            if (line.rfind("AT", 0) == 0) {  // Si la línea comienza con "AT"
                atResponseBuffer = line + "\n";  // Iniciar el buffer AT
                capturingAT = true; 
            }
            else if (capturingAT) {
                atResponseBuffer += line + "\n";
                
                if (line == "OK" || line == "ERROR") {
                    ESP_LOGI("SIM7600", "Respuesta AT completa: [%s]", atResponseBuffer.c_str());
                    std::string response = atResponseBuffer;
                    processLine(atResponseBuffer);
                    atResponseBuffer.clear();
                    capturingAT = false; // Resetear estado
                    ESP_LOGI("SIM7600", "STRING => [%s]", response.c_str());
                    return response;
                }
            }
            else if (line.find("+CGNSSINFO:") != std::string::npos) {
                //ESP_LOGI("SIM7600", "Evento GPS detectado");
                processEvent(line, "GPS");
            }
            else if (line.find("+CPSI:") != std::string::npos) {
                //ESP_LOGI("SIM7600", "Evento PIS detectado");
                processEvent(line, "PSI");
            } 
            else if (line.find("+CMTI:") != std::string::npos) {
                ESP_LOGI("SIM7600", "Evento SMS detectado");
                processEvent(line, "SMS");
            }
        }
    }
    return "";
}
void SIM7600::processEvent(const std::string& line, const std::string& eventType) {
    char buffer[256];
    
    if (eventType == "GPS" && line.find("+CGNSSINFO:") != std::string::npos) {

        std::string gpsData = line.substr(line.find("+CGNSSINFO: ") + 11);
        std::string cleanGpsData = cleanATResponse(line, "AT+CGNSSINFO?");
        if (!cleanGpsData.empty()) {
          gnssInfo gnssData = gps.parseGNSS(cleanGpsData);
          if (!xQueueSend(gps_queue, &gnssData, pdMS_TO_TICKS(100))) {
            ESP_LOGE("SIM7600", "Error: No se pudo enviar gnss data a la cola");
          }
        } else {
            ESP_LOGI("SIM7600", "Evento PSI CON ERROR %s", cleanGpsData.c_str());
        }

    } else if (eventType == "PSI" && line.find("+CPSI:") != std::string::npos) {
        std::string cleanGprsData = cleanATResponse(line, "AT+CPSI?");
        if (!cleanGprsData.empty()) {
            //ESP_LOGI("SIM7600", "PSI LIMPIO:%s", cleanGprsData.c_str());
            /*strncpy(buffer, cleanGprsData.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';
            xQueueSend(cell_queue, &buffer, pdMS_TO_TICKS(100));*/
            antennaInfo cellData = gprs.parseCPSI(cleanGprsData);
            if (!xQueueSend(cell_queue, &cellData, pdMS_TO_TICKS(100))) {
                ESP_LOGE("SIM7600", "Error: No se pudo enviar cellular data a la cola");
            }
        } else {
            ESP_LOGI("SIM7600", "Evento PSI CON ERROR %s", cleanGprsData.c_str());
        }    
    } else if (eventType == "SMS" && line.find("+CMTI:") != std::string::npos) {
        ESP_LOGI("SIM7600", "Nuevo SMS detectado");

        size_t pos = line.find(",");
        if (pos != std::string::npos && pos + 1 < line.size() ) {
            std::string index = line.substr(pos + 1);
            strncpy(buffer, index.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';
            index.erase(std::remove_if(index.begin(), index.end(), ::isspace), index.end());  // Limpia espacios
            ESP_LOGI("SIM7600", "Encolando índice de SMS: %s", index.c_str());
            xQueueSend(evt_queue, &buffer, portMAX_DELAY);
        }
    }
}
void SIM7600::processLine(const std::string& line) {

    if (line.find("+CMGR:") != std::string::npos) {
        SMSData sms;
        if (parseCMGR(line, sms)) {
            ESP_LOGI("SIM7600", "SMS Parseado y encolado.");
            xQueueSend(sms_queue, &sms, portMAX_DELAY);
        } else {
            ESP_LOGW("SIM7600", "Fallo al parsear SMS.");
        }
    } else if(line.find("+SIMEI:") != std::string::npos) {
        sm.imei = extractIMEI(cleanATResponse(line, "AT+SMEI?") );
        ESP_LOGW("SIM7600", "IMEI => %s", sm.imei.c_str());
    }
}
bool SIM7600::parseSMSCommand(const std::string& sms, std::string& imei, int& paramID, std::string& paramValue) {
    char cmd[4], imeiBuffer[16], paramBuffer[32];

    if (sscanf(sms.c_str(), "%3s;%15[^;];%d;%31s", cmd, imeiBuffer, &paramID, paramBuffer) == 4) {
        if (strcmp(cmd, "CMD") == 0) {
            ESP_LOGW("SIM7600", "Comando procesado.");
            imei = imeiBuffer;
            paramValue = paramBuffer;
            return true;
        }
    }
    return false;
}
std::string SIM7600::generateATCommand(int paramID, const std::string& paramValue) {
    ESP_LOGW("SIM7600", "Creando el comandoAT");
    switch (paramID) {
        case 0:
            return "AT+CGDCONT=1,\"IP\",\"" + paramValue + "\"";
        case 1: {
            size_t pos = paramValue.find(':');
            if (pos != std::string::npos) {
                std::string ip = paramValue.substr(0, pos);
                std::string port = paramValue.substr(pos + 1);
                return "AT+CIPOPEN=0,\"TCP\",\"" + ip + "\"," + port;
            }
            return "";
        }
        case 2:
            return (paramValue == "01") ? "AT+OUTPUT1=1" : "AT+OUTPUT1=0";
        default:
            return "";
    }
}
std::string SIM7600::cleanATResponse(const std::string& response, const std::string& command) {
    std::string result = response;
    size_t pos = result.find(command);
    if (pos != std::string::npos) {
        result.erase(pos, command.length());
    }
    pos = result.find(": ");
    if (pos != std::string::npos) {
        result.erase(0, pos + 2);
    }
    std::string cleaned_result;
    bool add_comma = false;
    for (char c : result) {
        if (c == '\n' || c == '\r') {
            add_comma = true;
        } else {
            if (add_comma && !cleaned_result.empty() && cleaned_result.back() != ',') {
                cleaned_result += ',';
            }
            cleaned_result += c;
            add_comma = false;
        }
    }
    return cleaned_result;

}
bool SIM7600::parseCMGR(const std::string& response, SMSData& sms) {
    std::string cleanResponse = cleanATResponse(response, "AT+CMGR=");
    // Buscar la cabecera "+CMGR:"
    //size_t pos = cleanResponse.find("+CMGR:");
    // Variables temporales
    char state[16] = {0}, phone[32] = {0}, datetime[32] = {0};
    char message[256] = {0}, status[16] = {0};

    // Extraer todos los valores de la cadena, incluyendo el estado final
    if (sscanf(cleanResponse.c_str(), "\"%15[^\"]\",\"%31[^\"]\",\"\",\"%31[^\"]\",%255[^,],%15s", 
               state, phone, datetime, message, status) == 5) {

        sms.state_sms = state;
        sms.phone_sms = phone;
        sms.datetime_sms = datetime;
        sms.data_sms = message;
        sms.state_cmd_at = status;  // Ahora extraemos directamente el "OK" o lo que venga

        ESP_LOGI("SIM7600", "SMS Parseado -> Estado: %s, Tel: %s, Fecha: %s, Mensaje: %s, Estado CMD: %s",
                 sms.state_sms.c_str(), sms.phone_sms.c_str(), sms.datetime_sms.c_str(), sms.data_sms.c_str(), sms.state_cmd_at.c_str());

        return true;
    }

    ESP_LOGW("SIM7600", "Error al parsear respuesta de AT+CMGR: %s", cleanResponse.c_str());
    return false;
}
