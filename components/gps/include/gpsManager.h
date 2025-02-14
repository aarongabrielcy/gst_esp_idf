#ifndef GPSMANAGER_H
#define GPSMANAGER_H

#include <vector>
#include <sstream>
#include <string>
#include "esp_log.h"
#include "gnssInfo.h"

class gpsManager {
    private:
    public:
      gnssInfo parseGNSS(const std::string& response);    

    
};
#endif