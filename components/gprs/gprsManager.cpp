#include "gprsManager.h"

gprsManager::gprsManager(SIM7600& sim7600) : simModule(sim7600) {}

bool gprsManager::initializeModule() {
  std::string at = simModule.sendCommandWithResponse("AT", 4000);
  ESP_LOGI("GPRSManager", "response=%s", at.c_str());
  if(at == "OK") {
    return true;
  }
  return false;
}