#include "gpsManager.h"

gpsManager::gpsManager(SIM7600& sim7600) : simModule(sim7600) {}

void gpsManager::activeGps(int state) {
  simModule.sendATCommand("AT+CGPS="+ std::to_string(state));
}
void gpsManager::confiGpsReports(int interval) {
  simModule.sendATCommand("AT+CGNSSINFO="+ std::to_string(interval));  
}