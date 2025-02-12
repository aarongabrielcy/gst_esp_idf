#ifndef ANTENNAINFO_H
#define ANTENNAINFO_H

#include <string>

struct antennaInfo {
    std::string systemMode;
    std::string operationMode;
    std::string mcc;
    std::string mnc;
    std::string lac;
    std::string cellId;
    int rsrq;  // RSRQ
    int rxLevel;
    int rssi;
    int rssnr;
};

#endif  // ANTENNAINFO_H