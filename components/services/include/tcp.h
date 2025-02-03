#ifndef TCP_H
#define TCP_H

#include "SIM7600.h"
#include <string> 
#include "esp_log.h"
#include "system_config.h"

class tcp {

private:
    SIM7600& simModule;
   
public:
    tcp(SIM7600& sim7600);
    void activeTcpService();
    void configTcpServer(const std::string& server, int port);
};

#endif