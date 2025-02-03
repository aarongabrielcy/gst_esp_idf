#ifndef SIM7600_H
#define SIM7600_H

#include <string>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdio.h>
#include "esp_timer.h"
#include "hardware_config.h"
#include "freertos/queue.h"
#include "freertos/task.h"

class SIM7600 {
public:
    SIM7600(uart_port_t uart_num);
    QueueHandle_t gps_queue;
    QueueHandle_t network_queue;
    QueueHandle_t sms_queue;
    void begin();
    void sendATCommand(const std::string& command);
    std::string readResponse();
    std::string sendCommandWithResponse(const std::string& command, int timeout);
    int commandType(const std::string& command);
    std::string processResponse(const std::string& command,  const std::string& fcommand, const std::string& response);
    void readUART();

private:
    uart_port_t _uart_num;
    void processLine(const std::string& line);

};
#endif
