#ifndef INITCONFIG_H
#define INITCONFIG_H

#include "SIM7600.h"

class initConfig {
    private:
        SIM7600& simModule;
    public:
        initConfig(SIM7600& sim7600);
        void activeGps(int state);
        void gpsReport(int interval);
        void gprsReport(int interval);
};
#endif