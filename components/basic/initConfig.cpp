#include "initConfig.h"

initConfig::initConfig(SIM7600& sim7600) : simModule(sim7600) {}

void initConfig::activeGps(int state) {
    simModule.sendATCommand("AT+CGPS="+ std::to_string(state));
}
void initConfig::gpsReport(int interval) {
    simModule.sendATCommand("AT+CGNSSINFO="+ std::to_string(interval));  
}
void initConfig::gprsReport(int interval) {
    simModule.sendATCommand("AT+CPSI="+ std::to_string(interval));  
}