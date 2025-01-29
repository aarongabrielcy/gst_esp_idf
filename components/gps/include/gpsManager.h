#ifndef GPSMANAGER_H
#define GPSMANAGER_H

#include "SIM7600.h"
#include <string>

class gpsManager {
    private:
        SIM7600& simModule;
    public:
        gpsManager(SIM7600& sim7600);
        void activeGps(int state);
        void confiGpsReports(int interval);


};
#endif