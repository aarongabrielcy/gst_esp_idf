#include "SIM7600.h"
#include <string.h>

SIM7600::SIM7600(uart_port_t uart_num) : _uart_num(uart_num) {
    gps_queue = xQueueCreate(10, sizeof(char) * 256);
    network_queue = xQueueCreate(10, sizeof(char) * 256);
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
}

void SIM7600::sendATCommand(const std::string& command) {
    std::string atCommand = command + "\r\n";  // Agregar retorno de carro y nueva línea
    uart_write_bytes(_uart_num, atCommand.c_str(), atCommand.length());
    ESP_LOGI("SIM7600", "Comando AT enviado: %s", atCommand.c_str());
}

/*std::string SIM7600::readResponse() {
    uint8_t data[1024];
    int len = uart_read_bytes(_uart_num, data, sizeof(data) - 1, pdMS_TO_TICKS(3000)); // Timeout 3 segundos
    if (len > 0) {
        data[len] = '\0';  // Finalizar la cadena
        ESP_LOGI("SIM7600", "Respuesta recibida: %s", data);
        return std::string((char*)data);
    } else {
        ESP_LOGW("SIM7600", "No se recibió respuesta.");
        return "Sin respuesta";
    }
}*/

/*std::string SIM7600::sendCommandWithResponse(const std::string& command, int timeout) {
    std::string atCommand = command + "\r\n";  // Agregar retorno de carro y nueva línea
    uart_write_bytes(_uart_num, atCommand.c_str(), atCommand.length());
    ESP_LOGI("SIM7600", "Comando AT enviado: %s", atCommand.c_str());

    std::string response = "";
    uint8_t data[1024];
    long startTime = esp_timer_get_time() / 1000; // Tiempo en milisegundos

    while ((esp_timer_get_time() / 1000) - startTime < timeout) {
        int len = uart_read_bytes(_uart_num, data, sizeof(data) - 1, pdMS_TO_TICKS(100)); // Leer con timeout corto
        if (len > 0) {
            data[len] = '\0'; // Terminar la cadena correctamente
            response += std::string((char*)data);
        }
    }

    if (response.empty()) {
        ESP_LOGW("SIM7600", "No se recibió respuesta.");
        return "Sin respuesta";
    }

    ESP_LOGI("SIM7600", "Respuesta recibida => %s", response.c_str());
    return response;
}*/
//AGREGA MILIs() A READUART SEGUNDO EQUIVALENTE EN sendCommandWithResponse
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

            ESP_LOGI("SIM7600", "Línea procesada: [%s]", line.c_str());

            if (line.rfind("AT", 0) == 0) {  // Si la línea comienza con "AT"
                atResponseBuffer = line + "\n";  // Iniciar el buffer AT
                capturingAT = true; 
            }
            else if (capturingAT) {
                atResponseBuffer += line + "\n";
                
                if (line == "OK" || line == "ERROR") {
                    ESP_LOGI("SIM7600", "Respuesta AT completa: [%s]", atResponseBuffer.c_str());
                    processLine(atResponseBuffer);
                    atResponseBuffer.clear();
                    capturingAT = false; // Resetear estado
                }
            }
            else if (line.find("+CGNSSINFO:") != std::string::npos) {
                ESP_LOGI("SIM7600", "Evento GPS detectado");
                processEvent(line, "GPS");
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
        ESP_LOGI("SIM7600", "Evento GPS detectado");

        std::string gpsData = line.substr(line.find("+CGNSSINFO:") + 11);
        strncpy(buffer, gpsData.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        xQueueSend(gps_queue, &buffer, portMAX_DELAY);
    } 
    
    else if (eventType == "SMS" && line.find("+CMTI:") != std::string::npos) {
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
    char buffer[256];

    /*if (line.find("+CGNSSINFO:") != std::string::npos) {
        strncpy(buffer, line.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        xQueueSend(gps_queue, &buffer, portMAX_DELAY);
    }*/ 
    if (line.find("+CPSI:") != std::string::npos) {
        strncpy(buffer, line.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        xQueueSend(network_queue, &buffer, portMAX_DELAY);
    } 
    else if (line.find("+CMGR:") != std::string::npos) {
        SMSData sms;
        if (parseCMGR(line, sms)) {
            ESP_LOGI("SIM7600", "SMS Parseado y encolado.");
            xQueueSend(sms_queue, &sms, portMAX_DELAY);
        } else {
            ESP_LOGW("SIM7600", "Fallo al parsear SMS.");
        }
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
bool SIM7600::parseCPSI(const std::string& response, CellData& cellData) {
    //ESP_LOGW("SIM7600", "Respuesta CPSI => %s", response.c_str());
    std::string cleanResponse = cleanATResponse(response, "AT+CPSI?");
    ESP_LOGW("SIM7600", "RespLimpia CPSI: %s", cleanResponse.c_str());       
    return false;
}