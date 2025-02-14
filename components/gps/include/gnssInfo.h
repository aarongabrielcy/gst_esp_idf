#ifndef GNSSINFO_H
#define GNSSINFO_H

#include <string>

struct gnssInfo {
    int mode = 0;
    int gps_svs = 0;
    int glonass_svs = 0;
    int beidou_svs = 0;
    double lat = 0.0;
    char ns = 'N';
    double log = 0.0;
    char ew = 'E';
    std::string date = "000000";
    std::string utc_time = "000000.0";
    double alt = 0.0;
    double speed = 0.0;
    double course = 0.0;
    double pdop = 0.0;
    double hdop = 0.0;
    double vdop = 0.0;
    int fix = 0;
};

#endif  // GNSSINFO_H