#ifndef GPRSMANAGER_H
#define GPRSMANAGER_H

#include "SIM7600.h"
#include <string>
#include "esp_log.h"

class gprsManager {
    private:
        SIM7600& simModule;
    public:
        gprsManager(SIM7600& sim7600);
        bool initializeModule();      
};
#endif