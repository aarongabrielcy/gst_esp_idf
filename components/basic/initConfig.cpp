#include "initConfig.h"

initConfig::initConfig(SIM7600& sim7600) : simModule(sim7600) {}

bool initConfig::activeGps(int state) {
    std::string response = simModule.sendATCommand("AT+CGPS="+ std::to_string(state),3000, 0);
    if (response.find("OK") != std::string::npos) {
        ESP_LOGI("INIT CONFIG", "GPS ACTIVADO CORRECTAMENTE");
        return true;
    }else {
        ESP_LOGI("INIT CONFIG", "Error al ACTIVAR EL GPS=>%s", response.c_str());
        return false;
    }
}
void initConfig::gpsReport(int interval) {
    simModule.sendATCommand("AT+CGNSSINFO="+ std::to_string(interval),3000, 0);  
}
void initConfig::gprsReport(int interval) {
    simModule.sendATCommand("AT+CPSI="+ std::to_string(interval),3000, 0);  
}