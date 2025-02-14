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
#include "gprsManager.h"
#include "antennaInfo.h"
#include "gpsManager.h"
#include "gnssInfo.h"
#include "moduleInfo.h"

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
    QueueHandle_t cell_queue;
    QueueHandle_t sms_queue;
    QueueHandle_t evt_queue;
    void begin();
    void sendATCommand(const std::string& command);
    /*std::string readResponse();
    std::string sendCommandWithResponse(const std::string& command, int timeout);*/
    std::string readUART();
     bool parseSMSCommand(const std::string& sms, std::string& imei, int& paramID, std::string& paramValue);
    std::string generateATCommand(int paramID, const std::string& paramValue);
    void getImei();

private:
    uart_port_t _uart_num;
    gprsManager gprs;
    gpsManager gps;
    bool testUART();
    std::string cleanATResponse(const std::string& response, const std::string& command);
    void processLine(const std::string& line);
    bool parseCMGR(const std::string& response, SMSData& sms);
    void processEvent(const std::string& line, const std::string& eventType);
};
#endif
