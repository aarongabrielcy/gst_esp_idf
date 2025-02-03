#include "SIM7600.h"

SIM7600::SIM7600(uart_port_t uart_num) : _uart_num(uart_num) {}

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

std::string SIM7600::readResponse() {
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
}

std::string SIM7600::sendCommandWithResponse(const std::string& command, int timeout) {
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

    ESP_LOGI("SIM7600", "Respuesta recibida: %s", response.c_str());
    return response;
}
int SIM7600::commandType(const std::string& command) {
    /*
    debe validar si es un comando interno o externo ej.
    caso 1: empieza con "AT+" es interno
    caso 2: empiesa con "CMD" es externo
    entonces segun sea el caso procesa el comando
    1. en el primer caso separa "AT+" del comando y después de enviar el comando al modulo limpia la respuesta quitando espacios en blanco saltos de linea y el mismo comando mandado.
    2. en el segundo caso deber validar después de las letras "CMD" Sigue el imei y debe balidar que sea el mismo del device y despues sigue el comando 00;03 que va a 
    desencadenar una accion segun el comando que sea;
    */
  return 0;
}
std::string SIM7600::processResponse(const std::string& command,  const std::string& fcommand, const std::string& response) {
    return "";
}