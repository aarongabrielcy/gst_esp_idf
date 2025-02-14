#ifndef GPRSMANAGER_H
#define GPRSMANAGER_H

//#include "SIM7600.h"
#include <string>
#include <iostream>
#include <regex>
#include "esp_log.h"
#include "antennaInfo.h"
#include <sstream>
#include <vector>

class gprsManager {
    private:
        //SIM7600& simModule;
        void parseGSM(const std::string& response, antennaInfo& cell); 
        void parseWCDMA(const std::string& response, antennaInfo& cell);
        void parseLTE(const std::string& response, antennaInfo& cell);
        void parseCDMA(const std::string& response, antennaInfo& cell);
        void parseEVDO(const std::string& response, antennaInfo& cell);
        static const char* systemMode[6];

    public:
        //gprsManager(SIM7600& sim7600);
        antennaInfo parseCPSI(const std::string& response);    
          
};
#endif