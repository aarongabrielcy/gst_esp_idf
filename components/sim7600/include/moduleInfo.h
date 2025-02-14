#ifndef MODULEINFO_H
#define MODULEINFO_H
#include <string>

struct moduleInfo {
    std::string imei;
    std::string ccid;
    std::string fw_sm = "1.0.1";
    std::string model_sm = "01";
};
extern moduleInfo sm;
#endif

