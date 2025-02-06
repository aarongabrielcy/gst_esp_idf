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
#include <algorithm>

struct SMSData {
    std::string state_sms;
    std::string phone_sms;
    std::string datetime_sms;
    std::string data_sms;
    std::string state_cmd_at;
};
class SIM7600 {
public:
    SIM7600(uart_port_t uart_num);
    QueueHandle_t gps_queue;
    QueueHandle_t network_queue;
    QueueHandle_t sms_queue;
    QueueHandle_t evt_queue;
    void begin();
    void sendATCommand(const std::string& command);
    /*std::string readResponse();
    std::string sendCommandWithResponse(const std::string& command, int timeout);*/
    std::string readUART();
     bool parseSMSCommand(const std::string& sms, std::string& imei, int& paramID, std::string& paramValue);
    std::string generateATCommand(int paramID, const std::string& paramValue);

    struct CellData {
        std::string sys_mode;
        std::string oper_mode;
        std::string mcc;
        std::string mnc;
        std::string lac;
        std::string cellID;
        std::string rxLev;
    };
private:
    uart_port_t _uart_num;
    std::string cleanATResponse(const std::string& response, const std::string& command);
    void processLine(const std::string& line);
    bool parseCMGR(const std::string& response, SMSData& sms);
    bool parseCPSI(const std::string& response, CellData& cell);
    void processEvent(const std::string& line, const std::string& eventType);
};
#endif
