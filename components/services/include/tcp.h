#ifndef TCP_H
#define TCP_H

#include "SIM7600.h"
/*#include <string> 
#include "esp_log.h"
#include "system_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"*/
class tcp {

private:
    SIM7600& simModule;
   
public:
    tcp(SIM7600& sim7600);
    bool activeTcpService();
    bool validConfigTCP();
    bool configTcpServer(const std::string& server, int port);
    bool sendData(std::string& message);
};

#endif