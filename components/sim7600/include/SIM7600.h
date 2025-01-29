#ifndef SIM7600_H
#define SIM7600_H

#include <string>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hardware_config.h"
class SIM7600 {
public:
    SIM7600(uart_port_t uart_num);
    void begin();
    void sendATCommand(const std::string& command);
    std::string readResponse();

private:
    uart_port_t _uart_num;
};
#endif
